
#pragma once




#include "Authenticator.h"





class cThread;
class cMonsterConfig;
class cGroupManager;
class cCraftingRecipes;
class cFurnaceRecipe;
class cWebAdmin;
class cPluginManager;
class cServer;
class cWorld;
class cPlayer;
typedef cItemCallback<cPlayer> cPlayerListCallback;
typedef cItemCallback<cWorld>  cWorldListCallback;





class cRoot	// tolua_export
{			// tolua_export
public:
	/// The version of the protocol that is primary for the server (reported in the server list). All versions are still supported.
	int m_PrimaryServerVersion;  // tolua_export
	
	static cRoot* Get() { return s_Root; }							// tolua_export

	cRoot(void);
	~cRoot();

	void Start(void);

	cServer * GetServer(void) { return m_Server; }						// tolua_export
	cWorld *  GetDefaultWorld(void);										// tolua_export
	cWorld *  GetWorld(const AString & a_WorldName);				// tolua_export
	
	/// Calls the callback for each world; returns true if the callback didn't abort (return true)
	bool ForEachWorld(cWorldListCallback & a_Callback);  // >> Exported in ManualBindings <<
	
	/// Logs chunkstats for each world and totals
	void LogChunkStats(void);
	
	int GetPrimaryServerVersion(void) const { return m_PrimaryServerVersion; }  // tolua_export
	void SetPrimaryServerVersion(int a_Version) { m_PrimaryServerVersion = a_Version; }  // tolua_export
	
	cMonsterConfig * GetMonsterConfig() { return m_MonsterConfig; }

	cGroupManager *    GetGroupManager   (void) { return m_GroupManager; }     // tolua_export
	cCraftingRecipes * GetCraftingRecipes(void) { return m_CraftingRecipes; }  // tolua_export
	cFurnaceRecipe *   GetFurnaceRecipe  (void) { return m_FurnaceRecipe; }    // tolua_export
	cWebAdmin *        GetWebAdmin       (void) { return m_WebAdmin; }         // tolua_export
	cPluginManager *   GetPluginManager  (void) { return m_PluginManager; }    // tolua_export
	cAuthenticator &   GetAuthenticator  (void) { return m_Authenticator; }

	/// Executes a console command through the cServer class; does special handling for "stop" and "restart".
	void ExecuteConsoleCommand(const AString & a_Cmd);						// tolua_export
	
	/// Kicks the user, no matter in what world they are. Used from cAuthenticator
	void KickUser(int a_ClientID, const AString & a_Reason);
	
	/// Called by cAuthenticator to auth the specified user
	void AuthenticateUser(int a_ClientID);

	void TickWorlds(float a_Dt);
	
	/// Returns the number of chunks loaded
	int GetTotalChunkCount(void);  // tolua_export
	
	/// Saves all chunks in all worlds
	void SaveAllChunks(void);  // tolua_export
	
	/// Calls the callback for each player in all worlds
	bool ForEachPlayer(cPlayerListCallback & a_Callback);	// >> EXPORTED IN MANUALBINDINGS <<

	/// Finds a player from a partial or complete player name and calls the callback - case-insensitive
	bool FindAndDoWithPlayer(const AString & a_PlayerName, cPlayerListCallback & a_Callback);	// >> EXPORTED IN MANUALBINDINGS <<
	
	/// Returns the textual description of the protocol version: 49 -> "1.4.4". Provided specifically for Lua API
	static AString GetProtocolVersionTextFromInt(int a_ProtocolVersionNum);  // tolua_export
	
private:
	void LoadGlobalSettings();

	/// Loads the worlds from settings.ini, creates the worldmap
	void LoadWorlds(void);
	
	/// Starts each world's life
	void StartWorlds(void);
	
	/// Stops each world's threads, so that it's safe to unload them
	void StopWorlds(void);
	
	/// Unloads all worlds from memory
	void UnloadWorlds(void);

	cServer *        m_Server;
	cMonsterConfig * m_MonsterConfig;

	cGroupManager *    m_GroupManager;
	cCraftingRecipes * m_CraftingRecipes;
	cFurnaceRecipe *   m_FurnaceRecipe;
	cWebAdmin *        m_WebAdmin;
	cPluginManager *   m_PluginManager;
	cAuthenticator     m_Authenticator;

	cMCLogger *      m_Log;

	bool m_bStop;
	bool m_bRestart;

	typedef std::map< AString, cWorld* > WorldMap;
	cWorld*  m_pDefaultWorld;
	WorldMap m_WorldsByName;

	cThread* m_InputThread;
	static void InputThread(void* a_Params);

	static cRoot*	s_Root;
};	// tolua_export




