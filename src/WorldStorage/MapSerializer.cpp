
// MapSerializer.cpp


#include "Globals.h"
#include "MapSerializer.h"
#include "OSSupport/GZipFile.h"
#include "FastNBT.h"

#include "../Map.h"
#include "../World.h"





cMapSerializer::cMapSerializer(const AString & a_WorldName, cMap * a_Map):
	m_Map(a_Map)
{
	auto DataPath = fmt::format(FMT_STRING("{}{}data"), a_WorldName, cFile::PathSeparator());
	m_Path = fmt::format(FMT_STRING("{}{}map_{}.dat"), DataPath, cFile::PathSeparator(), a_Map->GetID());
	cFile::CreateFolder(DataPath);
}





bool cMapSerializer::Load()
{
	const auto Data = GZipFile::ReadRestOfFile(m_Path);
	const cParsedNBT NBT(Data.GetView());

	if (!NBT.IsValid())
	{
		// NBT Parsing failed
		return false;
	}

	return LoadMapFromNBT(NBT);
}





bool cMapSerializer::Save(void)
{
	cFastNBTWriter Writer;
	SaveMapToNBT(Writer);
	Writer.Finish();

	#ifndef NDEBUG
	cParsedNBT TestParse(Writer.GetResult());
	ASSERT(TestParse.IsValid());
	#endif  // !NDEBUG

	GZipFile::Write(m_Path, Writer.GetResult());

	return true;
}





void cMapSerializer::SaveMapToNBT(cFastNBTWriter & a_Writer)
{
	a_Writer.BeginCompound("data");

	a_Writer.AddByte("scale", static_cast<Byte>(m_Map->GetScale()));
	a_Writer.AddByte("dimension", static_cast<Byte>(m_Map->GetDimension()));

	a_Writer.AddShort("width",  static_cast<Int16>(m_Map->GetWidth()));
	a_Writer.AddShort("height", static_cast<Int16>(m_Map->GetHeight()));

	a_Writer.AddInt("xCenter", m_Map->GetCenterX());
	a_Writer.AddInt("zCenter", m_Map->GetCenterZ());

	const cMap::cColorList & Data = m_Map->GetData();
	a_Writer.AddByteArray("colors", reinterpret_cast<const char *>(Data.data()), Data.size());

	a_Writer.EndCompound();
}





bool cMapSerializer::LoadMapFromNBT(const cParsedNBT & a_NBT)
{
	int Data = a_NBT.FindChildByName(0, "data");
	if (Data < 0)
	{
		return false;
	}

	int CurrLine = a_NBT.FindChildByName(Data, "scale");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_Byte))
	{
		unsigned int Scale = static_cast<unsigned int>(a_NBT.GetByte(CurrLine));
		m_Map->SetScale(Scale);
	}

	CurrLine = a_NBT.FindChildByName(Data, "dimension");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_Byte))
	{
		eDimension Dimension = static_cast<eDimension>(a_NBT.GetByte(CurrLine));

		if (Dimension != m_Map->m_World->GetDimension())
		{
			// TODO 2014-03-20 xdot: We should store nether maps in nether worlds, e.t.c.
			return false;
		}
	}

	CurrLine = a_NBT.FindChildByName(Data, "width");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_Short))
	{
		unsigned int Width = static_cast<unsigned int>(a_NBT.GetShort(CurrLine));
		if (Width != 128)
		{
			return false;
		}
		m_Map->m_Width = Width;
	}

	CurrLine = a_NBT.FindChildByName(Data, "height");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_Short))
	{
		unsigned int Height = static_cast<unsigned int>(a_NBT.GetShort(CurrLine));
		if (Height >= cChunkDef::Height)
		{
			return false;
		}
		m_Map->m_Height = Height;
	}

	CurrLine = a_NBT.FindChildByName(Data, "xCenter");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_Int))
	{
		int CenterX = a_NBT.GetInt(CurrLine);
		m_Map->m_CenterX = CenterX;
	}

	CurrLine = a_NBT.FindChildByName(Data, "zCenter");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_Int))
	{
		int CenterZ = a_NBT.GetInt(CurrLine);
		m_Map->m_CenterZ = CenterZ;
	}

	unsigned int NumPixels = m_Map->GetNumPixels();
	m_Map->m_Data.resize(NumPixels);

	CurrLine = a_NBT.FindChildByName(Data, "colors");
	if ((CurrLine >= 0) && (a_NBT.GetType(CurrLine) == TAG_ByteArray))
	{
		memcpy(m_Map->m_Data.data(), a_NBT.GetData(CurrLine), NumPixels);
	}

	return true;
}





cIDCountSerializer::cIDCountSerializer(const AString & a_WorldName) : m_MapCount(0)
{
	auto DataPath = fmt::format(FMT_STRING("{}{}data"), a_WorldName, cFile::PathSeparator());
	m_Path = fmt::format(FMT_STRING("{}{}idcounts.dat"), DataPath, cFile::PathSeparator());
	cFile::CreateFolder(DataPath);
}





bool cIDCountSerializer::Load()
{
	AString Data = cFile::ReadWholeFile(m_Path);
	if (Data.empty())
	{
		return false;
	}

	// NOTE: idcounts.dat is not compressed (raw format)

	// Parse the NBT data:
	cParsedNBT NBT({ reinterpret_cast<const std::byte *>(Data.data()), Data.size() });
	if (!NBT.IsValid())
	{
		// NBT Parsing failed
		return false;
	}

	int CurrLine = NBT.FindChildByName(0, "map");
	if (CurrLine >= 0)
	{
		m_MapCount = static_cast<unsigned int>(NBT.GetShort(CurrLine) + 1);
	}
	else
	{
		m_MapCount = 0;
	}

	return true;
}





bool cIDCountSerializer::Save(void)
{
	cFastNBTWriter Writer;

	if (m_MapCount > 0)
	{
		Writer.AddShort("map", static_cast<Int16>(m_MapCount - 1));
	}

	Writer.Finish();

	#ifndef NDEBUG
	cParsedNBT TestParse(Writer.GetResult());
	ASSERT(TestParse.IsValid());
	#endif  // !NDEBUG

	cFile File;
	if (!File.Open(m_Path, cFile::fmWrite))
	{
		return false;
	}

	// NOTE: idcounts.dat is not compressed (raw format)

	File.Write(Writer.GetResult().data(), Writer.GetResult().size());
	File.Close();

	return true;
}
