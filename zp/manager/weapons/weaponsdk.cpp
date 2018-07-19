/**
 * ============================================================================
 *
 *  Zombie Plague Mod #3 Generation
 *
 *  File:          weaponsdk.cpp
 *  Type:          Module
 *  Description:   Weapon alpha functions, and alpha updating on drop/pickup.
 *
 *  Copyright (C) 2015-2018 Nikita Ushakov (Ireland, Dublin). Regards to Andersso
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 **/

/**
 * Number of valid slots.
 **/
enum WeaponSDKSlotType
{ 
    SlotType_Invalid = -1,        /** Used as return value when an weapon doens't exist. */
    
    SlotType_Primary,             /** Primary slot */
    SlotType_Secondary,           /** Secondary slot */
    SlotType_Melee,               /** Melee slot */
    SlotType_Equipment,           /** Equipment slot */  
    SlotType_C4                   /** C4 slot */  
};

/**
 * Number of valid access.
 **/
enum WeaponSDKClassType
{ 
    ClassType_Invalid,           /** Used as return value when an access doens't exist. */
    
    ClassType_Human,             /** Human access */
    ClassType_Survivor,          /** Survivor access */
    ClassType_Zombie,            /** Zombie access */
    ClassType_Nemesis            /** Nemesis access */
};

/**
 * Variables to store SDK calls handlers.
 **/
Handle hSDKCallRemoveAllItems;
Handle hSDKCallWeaponSwitch;
Handle hSDKCallCSWeaponDrop;
Handle hSDKCallGetMaxClip1;
Handle hSDKCallGetReserveAmmoMax;

#if defined USE_DHOOKS
 /**
 * Variables to store DHook calls handlers.
 **/
Handle hDHookGetMaxClip;
Handle hDHookGetReserveAmmoMax;

/**
 * Variables to store dynamic DHook offsets.
 **/
int DHook_GetMaxClip1;
int DHook_GetReserveAmmoMax;
#endif

/**
 * Initialize the main virtual/dynamic offsets for the weapon SDK/DHook system.
 **/
void WeaponSDKInit(/*void*/) /// https://www.unknowncheats.me/forum/counterstrike-global-offensive/152722-dumping-datamap_t.html
{                             // C_BaseFlex -> C_EconEntity -> C_BaseCombatWeapon -> C_WeaponCSBase -> C_BaseCSGrenade
    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(gServerData[Server_GameConfig][Game_Zombie], SDKConf_Virtual, "Weapon_RemoveAllItems");

    // Validate call
    if(!(hSDKCallRemoveAllItems = EndPrepSDKCall()))
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_CORE_EVENTS, LogModule_Weapons, "GameData Validation", "Failed to load SDK call \"CBasePlayer::RemoveAllItems\". Update virtual offset in \"%s\"", PLUGIN_CONFIG);
    }

    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(gServerData[Server_GameConfig][Game_SDKHooks], SDKConf_Virtual, "Weapon_Switch");
    PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
    PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);

    // Validate call
    if(!(hSDKCallWeaponSwitch = EndPrepSDKCall()))
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_CORE_EVENTS, LogModule_Weapons, "GameData Validation", "Failed to load SDK call \"CBasePlayer::Weapon_Switch\". Update \"SourceMod\"");
    }

    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Player);
    PrepSDKCall_SetFromConf(gServerData[Server_GameConfig][Game_CStrike], SDKConf_Signature, "CSWeaponDrop");

    // Adds a parameter to the calling convention. This should be called in normal ascending order
    PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
    PrepSDKCall_AddParameter(SDKType_Bool, SDKPass_Plain);
    PrepSDKCall_AddParameter(SDKType_Bool, SDKPass_Plain);

    // Validate call
    if(!(hSDKCallCSWeaponDrop = EndPrepSDKCall()))
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_CORE_EVENTS, LogModule_Weapons, "GameData Validation", "Failed to load SDK call \"CBasePlayer::CSWeaponDrop\". Update \"SourceMod\"");
    }
    
    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Entity);
    PrepSDKCall_SetFromConf(gServerData[Server_GameConfig][Game_Zombie], SDKConf_Virtual, "Weapon_GetMaxClip1");
    PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_ByValue); 
    
    // Validate call
    if(!(hSDKCallGetMaxClip1 = EndPrepSDKCall()))
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_CORE_EVENTS, LogModule_Weapons, "GameData Validation", "Failed to load SDK call \"CBaseCombatWeapon::GetMaxClip1\". Update virtual offset in \"%s\"", PLUGIN_CONFIG);
    }
    
    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Entity);
    PrepSDKCall_SetFromConf(gServerData[Server_GameConfig][Game_Zombie], SDKConf_Virtual, "Weapon_GetReserveAmmoMax");
    PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_ByValue); 
    
    // Validate call
    if(!(hSDKCallGetReserveAmmoMax = EndPrepSDKCall()))
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_CORE_EVENTS, LogModule_Weapons, "GameData Validation", "Failed to load SDK call \"CBaseCombatWeapon::GetReserveAmmoMax\". Update virtual offset in \"%s\"", PLUGIN_CONFIG);
    }
    
    /*_________________________________________________________________________________________________________________________________________*/

    // Load offsets here
    fnInitSendPropOffset(g_iOffset_EntityEffects, "CBaseEntity", "m_fEffects");
    fnInitSendPropOffset(g_iOffset_EntityModelIndex, "CBaseEntity", "m_nModelIndex");
    fnInitSendPropOffset(g_iOffset_EntityOwnerEntity, "CBaseEntity", "m_hOwnerEntity");
    fnInitSendPropOffset(g_iOffset_WeaponOwner, "CBaseCombatWeapon", "m_hOwner");
    fnInitSendPropOffset(g_iOffset_WeaponWorldModel, "CBaseCombatWeapon", "m_hWeaponWorldModel"); /**"m_iWorldModelIndex" not flexible**/
    fnInitSendPropOffset(g_iOffset_WeaponBody, "CBaseCombatWeapon", "m_nBody");
    fnInitSendPropOffset(g_iOffset_WeaponSkin, "CBaseCombatWeapon", "m_nSkin");
    ///fnInitSendPropOffset(g_iOffset_WeaponParent, "CBaseWeaponWorldModel", "m_hCombatWeaponParent");
    fnInitSendPropOffset(g_iOffset_CharacterWeapons, "CBaseCombatCharacter", "m_hMyWeapons");
    fnInitSendPropOffset(g_iOffset_PlayerViewModel, "CBasePlayer", "m_hViewModel");
    fnInitSendPropOffset(g_iOffset_PlayerActiveWeapon, "CBasePlayer", "m_hActiveWeapon");
    fnInitSendPropOffset(g_iOffset_PlayerObserverMode, "CBasePlayer", "m_iObserverMode");
    fnInitSendPropOffset(g_iOffset_PlayerObserverTarget, "CBasePlayer", "m_hObserverTarget");
    fnInitSendPropOffset(g_iOffset_PlayerAddonBits, "CCSPlayer", "m_iAddonBits");
    fnInitSendPropOffset(g_iOffset_PlayerArms, "CCSPlayer", "m_szArmsModel");
    fnInitSendPropOffset(g_iOffset_PlayerAttack, "CBasePlayer", "m_flNextAttack");
    fnInitSendPropOffset(g_iOffset_ViewModelOwner, "CBaseViewModel", "m_hOwner");
    fnInitSendPropOffset(g_iOffset_ViewModelWeapon, "CBaseViewModel", "m_hWeapon");
    fnInitSendPropOffset(g_iOffset_ViewModelSequence, "CBaseViewModel", "m_nSequence");
    fnInitSendPropOffset(g_iOffset_ViewModelPlaybackRate, "CBaseViewModel", "m_flPlaybackRate");
    fnInitSendPropOffset(g_iOffset_ViewModelIndex, "CBaseViewModel", "m_nViewModelIndex");
    fnInitSendPropOffset(g_iOffset_ViewModelIgnoreOffsAcc, "CBaseViewModel", "m_bShouldIgnoreOffsetAndAccuracy");
    fnInitSendPropOffset(g_iOffset_EconItemDefinitionIndex, "CEconEntity", "m_iItemDefinitionIndex");
    fnInitSendPropOffset(g_iOffset_NewSequenceParity, "CBaseAnimating", "m_nNewSequenceParity");
    fnInitSendPropOffset(g_iOffset_LastShotTime, "CWeaponCSBase", "m_fLastShotTime");
    ///fnInitSendPropOffset(g_iOffset_PlayerAmmo, "CBasePlayer", "m_iAmmo"); 
    fnInitSendPropOffset(g_iOffset_WeaponAmmoType, "CBaseCombatWeapon", "m_iPrimaryAmmoType");
    fnInitSendPropOffset(g_iOffset_WeaponClip1, "CBaseCombatWeapon", "m_iClip1");
    ///fnInitSendPropOffset(g_iOffset_WeaponClip2, "CBaseCombatWeapon", "m_iClip2");
    fnInitSendPropOffset(g_iOffset_WeaponReserve1, "CBaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
    fnInitSendPropOffset(g_iOffset_WeaponReserve2, "CBaseCombatWeapon", "m_iSecondaryReserveAmmoCount");
    fnInitSendPropOffset(g_iOffset_GrenadeThrower, "CBaseGrenade", "m_hThrower");

    /*_________________________________________________________________________________________________________________________________________*/
    
    #if defined USE_DHOOKS
    // Load offsets
    fnInitGameConfOffset(gServerData[Server_GameConfig][Game_Zombie], DHook_GetMaxClip1, "Weapon_GetMaxClip1");
    fnInitGameConfOffset(gServerData[Server_GameConfig][Game_Zombie], DHook_GetReserveAmmoMax, "Weapon_GetReserveAmmoMax");

    /// CBaseCombatWeapon::GetMaxClip1(CBaseCombatWeapon *this)
    hDHookGetMaxClip = DHookCreate(DHook_GetMaxClip1, HookType_Entity, ReturnType_Int, ThisPointer_CBaseEntity, WeaponDHookOnGetMaxClip1);
    
    /// CBaseCombatWeapon::GetReserveAmmoMax(AmmoPosition_t)
    hDHookGetReserveAmmoMax = DHookCreate(DHook_GetReserveAmmoMax, HookType_Entity, ReturnType_Int, ThisPointer_CBaseEntity, WeaponDHookOnGetReverseMax);
    DHookAddParam(hDHookGetReserveAmmoMax, HookParamType_Unknown);
    #endif
}

/**
 * Restore weapon models during the unloading.
 **/
void WeaponSDKUnload(/*void*/)
{
    // i = client index
    for(int i = 1; i <= MaxClients; i++)
    {
        // Validate client
        if(IsPlayerExist(i) && gClientData[i][Client_CustomWeapon])
        {
            // Gets the entity index from the reference
            int viewModel1 = EntRefToEntIndex(gClientData[i][Client_ViewModels][0]);
            int viewModel2 = EntRefToEntIndex(gClientData[i][Client_ViewModels][1]);

            // Validate fist view model
            if(viewModel1 != INVALID_ENT_REFERENCE)
            {
                // Make the first view model visible
                WeaponHDRSetEntityVisibility(viewModel1, true);
                SDKCall(hSDKCallEntityUpdateTransmitState, viewModel1);
            }

            // Validate secondary view model
            if(viewModel2 != INVALID_ENT_REFERENCE)
            {
                // Make the second view model visible
                WeaponHDRSetEntityVisibility(viewModel2, false);
                SDKCall(hSDKCallEntityUpdateTransmitState, viewModel2);
            }
        }
    }
}

/**
 * Client is joining the server.
 * 
 * @param clientIndex       The client index.  
 **/
void WeaponSDKClientInit(int clientIndex)
{
    // Hook entity callbacks
    SDKHook(clientIndex, SDKHook_WeaponCanUse, WeaponSDKOnCanUse);
    SDKHook(clientIndex, SDKHook_WeaponSwitch, WeaponSDKOnDeploy);
    SDKHook(clientIndex, SDKHook_WeaponSwitchPost, WeaponSDKOnDeployPost);
    SDKHook(clientIndex, SDKHook_WeaponDrop , WeaponSDKOnDrop);
    SDKHook(clientIndex, SDKHook_PostThinkPost, WeaponSDKOnAnimationFix);
}

/**
 * Hook commands specific to buy ammunition. Called when commands are created.
 **/
void WeaponSDKOnCommandsCreate(/*void*/)
{
    // Hook commands
    AddCommandListener(WeaponSDKOnAmmunition, "buyammo1");
    AddCommandListener(WeaponSDKOnAmmunition, "buyammo2");
}

/**
 * Called, when an weapon is created.
 *
 * @param entityIndex       The entity index.
 * @param sClassname        The string with returned name.
 **/
void WeaponSDKOnCreated(int entityIndex, const char[] sClassname)
{
    // Validate weapon
    if(!strncmp(sClassname, "weapon_", 7, false))
    {
        // Hook weapon callbacks
        SDKHook(entityIndex, SDKHook_ReloadPost, WeaponSDKOnWeaponReload);
        SDKHook(entityIndex, SDKHook_SpawnPost, WeaponSDKOnWeaponSpawn);
    }
    else
    {
        // Gets string length
        int iLen = strlen(sClassname) - 11;
        
        // Validate length
        if(iLen)
        {
            // Validate grenade
            if(!strncmp(sClassname[iLen], "_proj", 5, false))
            {
                // Hook grenade callbacks
                SDKHook(entityIndex, SDKHook_SpawnPost, WeaponSDKOnGrenadeSpawn);
            }
        }
    }
}

/**
 * Hook: WeaponReloadPost
 * Weapon is reloaded.
 *
 * @param weaponIndex       The weapon index.
 **/
public Action WeaponSDKOnWeaponReload(int weaponIndex) 
{
    // Apply fake reload hook on the next frame
    RequestFrame(view_as<RequestFrameCallback>(WeaponSDKOnFakeWeaponReload), weaponIndex);
}

/**
 * FakeHook: WeaponReloadPost
 *
 * @param referenceIndex    The reference index.
 **/
public void WeaponSDKOnFakeWeaponReload(int referenceIndex) 
{
    // Get the weapon index from the reference
    int weaponIndex = EntRefToEntIndex(referenceIndex);

    // Validate weapon
    if(weaponIndex != INVALID_ENT_REFERENCE)
    {
        // Validate custom index
        int iD = WeaponsGetCustomID(weaponIndex);
        if(iD != INVALID_ENT_REFERENCE)
        {
            // If custom reload speed exist, then apply it
            float flReload = WeaponsGetReload(iD);
            if(flReload)
            {
                // Gets the weapon owner
                int clientIndex = GetEntDataEnt2(weaponIndex, g_iOffset_WeaponOwner);
                
                // Validate owner
                if(!IsPlayerExist(clientIndex)) 
                {
                    return;
                }
                
                // Sets the next attack time
                SetEntDataFloat(clientIndex, g_iOffset_PlayerAttack, GetGameTime() + flReload, true);
            }
        }
    }
}

/**
 * Hook: WeaponSpawnPost
 * Weapon is spawned.
 *
 * @param weaponIndex       The weapon index.
 **/
public void WeaponSDKOnWeaponSpawn(int weaponIndex)
{
    // Validate weapon
    if(IsValidEdict(weaponIndex)) 
    {
        // Reset the weapon id
        WeaponsSetCustomID(weaponIndex, INVALID_ENT_REFERENCE);
    }
    
    #if defined USE_DHOOKS
        // Apply fake spawn hook on the next frame
        RequestFrame(view_as<RequestFrameCallback>(WeaponSDKOnFakeSpawnPost), EntIndexToEntRef(weaponIndex));
    #endif
}

/**
 * FakeHook: WeaponSpawnPost
 *
 * @param referenceIndex    The reference index.
 **/
#if defined USE_DHOOKS
public void WeaponSDKOnFakeSpawnPost(int referenceIndex) 
{
    // Get the weapon index from the reference
    int weaponIndex = EntRefToEntIndex(referenceIndex);

    // Validate weapon
    if(weaponIndex != INVALID_ENT_REFERENCE)
    {
        // Validate custom index
        int iD = WeaponsGetCustomID(weaponIndex);
        if(iD != INVALID_ENT_REFERENCE)
        {
            // Gets the weapon clip
            int iClip = WeaponsGetClip(iD);
            if(iClip)
            {/// Set clip here, because of creating dhook on the next frame after spawn
                SetEntData(weaponIndex, g_iOffset_WeaponClip1, iClip, _, true); 
                DHookEntity(hDHookGetMaxClip, false, weaponIndex);
            }

            // Gets the weapon ammo
            int iAmmo = WeaponsGetAmmo(iD);
            if(iAmmo)
            {/// Set ammo here, because of creating dhook on the next frame after spawn
                SetEntData(weaponIndex, g_iOffset_WeaponReserve1, iAmmo, _, true); 
                DHookEntity(hDHookGetReserveAmmoMax, false, weaponIndex);
            }
        }
    }
}
#endif

/**
 * Hook: WeaponSpawnPost
 * Grenade is spawned.
 *
 * @param grenadeIndex       The grenade index.
 **/
public void WeaponSDKOnGrenadeSpawn(int grenadeIndex)
{
    // Validate grenade
    if(IsValidEdict(grenadeIndex)) 
    {
        // Reset the grenade id
        WeaponsSetCustomID(grenadeIndex, INVALID_ENT_REFERENCE);
    }
    
    // Apply fake throw hook on the next frame
    RequestFrame(view_as<RequestFrameCallback>(WeaponSDKOnFakeGrenadeSpawn), EntIndexToEntRef(grenadeIndex));
}

/**
 * FakeHook: WeaponSpawnPost
 *
 * @param referenceIndex    The reference index.
 **/
public void WeaponSDKOnFakeGrenadeSpawn(int referenceIndex) 
{
    // Get the grenade index from the reference
    int grenadeIndex = EntRefToEntIndex(referenceIndex);

    // Validate grenade for the prop
    if(grenadeIndex != INVALID_ENT_REFERENCE)
    {
        // Gets the grenade thrower
        int clientIndex = GetEntDataEnt2(grenadeIndex, g_iOffset_GrenadeThrower);
        
        // Validate thrower
        if(!IsPlayerExist(clientIndex)) 
        {
            return;
        }
        
        // Gets the active weapon index from the client
        int weaponIndex = GetEntDataEnt2(clientIndex, g_iOffset_PlayerActiveWeapon);
        
        // Validate weapon
        if(!IsValidEdict(weaponIndex))
        {
            return;
        }

        // Validate custom index
        int iD = WeaponsGetCustomID(weaponIndex);
        if(iD != INVALID_ENT_REFERENCE)
        {
            // Validate grenade slot
            if(WeaponsGetSlot(iD) == 6) /// (Bug fix for other addons)
            {
                // Duplicate index to the projectile for future use
                WeaponsSetCustomID(grenadeIndex, iD);
        
                // Call forward
                API_OnWeaponCreated(grenadeIndex, iD);
        
                // If custom weapons models disabled, then skip
                if(gCvarList[CVAR_GAME_CUSTOM_MODELS].BoolValue)
                {
                    // If world model exist, then apply it
                    if(WeaponsGetModelWorldID(iD))
                    {
                        // Gets weapon world model
                        static char sModel[PLATFORM_MAX_PATH];
                        WeaponsGetModelWorld(iD, sModel, sizeof(sModel));

                        // Sets the model entity for the grenade
                        SetEntityModel(grenadeIndex, sModel);
                        
                        // Sets the body/skin index for the grenade
                        SetEntData(grenadeIndex, g_iOffset_WeaponBody, WeaponsGetModelBody(iD), _, true);
                        SetEntData(grenadeIndex, g_iOffset_WeaponSkin, WeaponsGetModelSkin(iD), _, true);
                    }
                }
            }
        }
    }
}

/**
 * Hook: WeaponDrop
 * Player drop any weapon.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
public Action WeaponSDKOnDrop(int clientIndex, int weaponIndex)
{
    // If custom weapons models disabled, then skip
    if(gCvarList[CVAR_GAME_CUSTOM_MODELS].BoolValue)
    {
        // Validate weapon
        if(IsValidEdict(weaponIndex))
        {
            // Apply dropped model on the next frame
            RequestFrame(view_as<RequestFrameCallback>(WeaponHDRSetDroppedModel), EntIndexToEntRef(weaponIndex));
        }
    }
    
    // Allow drop
    return Plugin_Continue;
}

/**
 * Hook: WeaponCanUse
 * Player pick-up any weapon.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
public Action WeaponSDKOnCanUse(int clientIndex, int weaponIndex)
{
    // Validate weapon
    if(IsValidEdict(weaponIndex))
    {
        // Validate custom index
        int iD = WeaponsGetCustomID(weaponIndex);
        if(iD != INVALID_ENT_REFERENCE)
        {
            // Switch class access
            switch(WeaponsGetClass(iD))
            {
                // Validate human class
                case ClassType_Human : if(!(!gClientData[clientIndex][Client_Zombie] && !gClientData[clientIndex][Client_Survivor]))   return Plugin_Handled;
                
                // Validate survivor class
                case ClassType_Survivor : if(!(!gClientData[clientIndex][Client_Zombie] && gClientData[clientIndex][Client_Survivor])) return Plugin_Handled;
                
                // Validate zombie class
                case ClassType_Zombie : if(!(gClientData[clientIndex][Client_Zombie] && !gClientData[clientIndex][Client_Nemesis]))    return Plugin_Handled;
                
                // Validate nemesis class
                case ClassType_Nemesis : if(!(gClientData[clientIndex][Client_Zombie] && gClientData[clientIndex][Client_Nemesis]))    return Plugin_Handled;
            
                // Validate invalid class
                default: return Plugin_Handled;
            }

            /** Uncomment bellow, if you want to check the client level/online access **/
            
            // Block pickup it, if online too low
            /*
            if(!gClientData[clientIndex][Client_Survivor] && fnGetPlaying() < WeaponsGetOnline(iD))
            {
                return Plugin_Handled;
            }

            // Block pickup it, if level too low
            if(!gClientData[clientIndex][Client_Survivor] && gClientData[clientIndex][Client_Level] < WeaponsGetLevel(iD))
            {
                return Plugin_Handled;
            }
            */
        }
    }
    
    // Allow pickup
    return Plugin_Continue;
}

/**
 * Client has been changed class state. (Post)
 *
 * @param clientIndex       The client index.
 **/
void WeaponSDKOnClientUpdate(int clientIndex)
{
    // Clear the weapon index of the reference
    gClientData[clientIndex][Client_CustomWeapon] = 0;

    // Remove current addons
    WeaponAttachRemoveAddons(clientIndex);
    
    // Gets the player viewmodel indexes
    int viewModel1 = WeaponHDRGetPlayerViewModel(clientIndex, 0);
    int viewModel2 = WeaponHDRGetPlayerViewModel(clientIndex, 1);

    // If a secondary view model doesn't exist, create one
    if(!IsValidEdict(viewModel2))
    {
        // Validate entity
        if((viewModel2 = CreateEntityByName("predicted_viewmodel")) == INVALID_ENT_REFERENCE)
        {
            // Unexpected error, log it
            LogEvent(false, LogType_Error, LOG_CORE_EVENTS, LogModule_Weapons, "Weapons HDR", "Failed to create secondary view model");
            return;
        }

        // Sets owner to the entity
        SetEntDataEnt2(viewModel2, g_iOffset_ViewModelOwner, clientIndex, true);
        SetEntData(viewModel2, g_iOffset_ViewModelIndex, 1, _, true);

        // Remove accuracity
        SetEntData(viewModel2, g_iOffset_ViewModelIgnoreOffsAcc, true, 1, true);

        // Spawn the entity into the world
        DispatchSpawn(viewModel2);

        // Sets the view model to the owner
        WeaponHDRSetPlayerViewModel(clientIndex, 1, viewModel2);
    }

    // Sets the entity index to the reference
    gClientData[clientIndex][Client_ViewModels][0] = EntIndexToEntRef(viewModel1);
    gClientData[clientIndex][Client_ViewModels][1] = EntIndexToEntRef(viewModel2);

    // Gets the active weapon index from the client
    int weaponIndex = GetEntDataEnt2(clientIndex, g_iOffset_PlayerActiveWeapon);

    // Validate weapon
    if(IsValidEdict(weaponIndex))
    {
        // Update the weapon
        SetEntDataEnt2(clientIndex, g_iOffset_PlayerActiveWeapon, weaponIndex, true);
        SDKCall(hSDKCallWeaponSwitch, clientIndex, weaponIndex, 0);
    }
}

/**
 * Event: PlayerDeath
 * Update a weapon model when player died.
 *
 * @param clientIndex       The client index.
 **/
void WeaponSDKOnClientDeath(int clientIndex)
{
    // Gets the entity index from the reference
    int viewModel2 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][1]);

    // Validate secondary view model
    if(viewModel2 != INVALID_ENT_REFERENCE)
    {
        // Hide the custom view model if the player dies
        WeaponHDRSetEntityVisibility(viewModel2, false);
        SDKCall(hSDKCallEntityUpdateTransmitState, viewModel2);
    }

    // Clear the data
    gClientData[clientIndex][Client_ViewModels] = { INVALID_ENT_REFERENCE, INVALID_ENT_REFERENCE };
    gClientData[clientIndex][Client_CustomWeapon] = 0;
}

/**
 * Hook: WeaponSwitch
 * Player deploy any weapon.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
public void WeaponSDKOnDeploy(int clientIndex, int weaponIndex) 
{
    // Gets the entity index from the reference
    int viewModel1 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][0]);
    int viewModel2 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][1]);

    // Validate view models
    if(viewModel1 == INVALID_ENT_REFERENCE || viewModel2 == INVALID_ENT_REFERENCE)
    {
        return;
    }

    // If custom weapons models disabled, then skip
    if(gCvarList[CVAR_GAME_CUSTOM_MODELS].BoolValue)
    {
        // Validate weapon
        if(IsValidEdict(weaponIndex))
        {
            // Validate custom index
            int iD = WeaponsGetCustomID(weaponIndex);
            if(iD != INVALID_ENT_REFERENCE)
            {
                // If view/world model exist, then apply them later
                if(WeaponsGetModelViewID(iD) || WeaponsGetModelWorldID(iD) || ((WeaponsValidateKnife(weaponIndex) || WeaponsValidateGrenade(weaponIndex)) && gClientData[clientIndex][Client_Zombie]))
                {
                    // Client has swapped to a custom weapon
                    gClientData[clientIndex][Client_SwapWeapon] = INVALID_ENT_REFERENCE;
                    gClientData[clientIndex][Client_CustomWeapon] = weaponIndex;
                    gClientData[clientIndex][Client_WeaponIndex] = iD; /// Only view model identification
                    return;
                }
            }
        }
    }

    // Client has swapped to a regular weapon
    gClientData[clientIndex][Client_CustomWeapon] = 0;
    gClientData[clientIndex][Client_WeaponIndex] = INVALID_ENT_REFERENCE; /// Only view model identification
    
    // Gets human arm model
    static char sArm[PLATFORM_MAX_PATH];
    HumanGetArmModel(gClientData[clientIndex][Client_HumanClass], sArm, sizeof(sArm));
    
    // Apply arm model
    if(strlen(sArm)) SetEntDataString(clientIndex, g_iOffset_PlayerArms, sArm, sizeof(sArm), true);
}

/**
 * Hook: WeaponSwitchPost
 * Player deploy any weapon.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
public void WeaponSDKOnDeployPost(int clientIndex, int weaponIndex) 
{
    // Validate client
    if(!IsPlayerExist(clientIndex))
    {
        return;
    }
    
    // Validate weapon
    if(!IsValidEdict(weaponIndex))
    {
        return;
    }

    // Gets the entity index from the reference
    int viewModel1 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][0]);
    int viewModel2 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][1]);

    // Validate view models
    if(viewModel1 == INVALID_ENT_REFERENCE || viewModel2 == INVALID_ENT_REFERENCE)
    {
        return;
    }

    // Initialize variables
    int iModel; static char sModel[PLATFORM_MAX_PATH]; sModel[0] = '\0';
    
    // Gets the weapon id from the reference
    int iD = gClientData[clientIndex][Client_WeaponIndex]; /// Only view model identification

    // Validate zombie
    if(gClientData[clientIndex][Client_Zombie] && !gClientData[clientIndex][Client_Nemesis])
    {
        // Validate knife
        if(WeaponsValidateKnife(weaponIndex))
        {
            // Gets claw model
            ZombieGetClawModel(gClientData[clientIndex][Client_ZombieClass], sModel, sizeof(sModel));
            
            // Update model index
            iModel = ZombieGetClawID(gClientData[clientIndex][Client_ZombieClass]);
        }
        // Validate grenade
        else if(WeaponsValidateGrenade(weaponIndex))
        {
            // Gets grenade model
            ZombieGetGrenadeModel(gClientData[clientIndex][Client_ZombieClass], sModel, sizeof(sModel));
            
            // Update model index
            iModel = ZombieGetGrenadeID(gClientData[clientIndex][Client_ZombieClass]);
        }

        // Resets if model wasn't found 
        if(!iModel || !strlen(sModel))
        {
            // Clear the custom id cache
            gClientData[clientIndex][Client_CustomWeapon] = 0; iD = INVALID_ENT_REFERENCE; /// (Bug fix for zombie)
        }
    }

    // Validate the switched weapon
    if(weaponIndex != gClientData[clientIndex][Client_CustomWeapon])
    {
        // Hide the secondary view model. This needs to be done on post because the weapon needs to be switched first
        if(iD == INVALID_ENT_REFERENCE)
        {
            // Make the first view model visible
            WeaponHDRSetEntityVisibility(viewModel1, true);
            SDKCall(hSDKCallEntityUpdateTransmitState, viewModel1);

            // Make the second view model invisible
            WeaponHDRSetEntityVisibility(viewModel2, false);
            SDKCall(hSDKCallEntityUpdateTransmitState, viewModel2);

            // Resets the weapon index
            gClientData[clientIndex][Client_WeaponIndex] = INVALID_ENT_REFERENCE; /// Only view model identification
        }
        return;
    }
    
    // Gets the model index
    if(!iModel) iModel = WeaponsGetModelViewID(iD);
    
    // Gets weapon view model
    if(!strlen(sModel)) WeaponsGetModelView(iD, sModel, sizeof(sModel));   

    // If view model exist, then apply it
    if(iModel)
    {
        // Make the first view model invisible
        WeaponHDRSetEntityVisibility(viewModel1, false);
        SDKCall(hSDKCallEntityUpdateTransmitState, viewModel1);
        
        // Make the second view model visible
        WeaponHDRSetEntityVisibility(viewModel2, true);
        ///SDKCall(hSDKCallEntityUpdateTransmitState, viewModel2); //-> transport a bit below
        
        // Remove the muzzle on the switch
        VEffectRemoveMuzzle(clientIndex, viewModel2);

        // Gets the draw animation sequence
        gClientData[clientIndex][Client_DrawSequence] = GetEntData(viewModel1, g_iOffset_ViewModelSequence);
        
        // Switch to an invalid sequence to prevent it from playing sounds before UpdateTransmitStateTime() is called
        SetEntData(viewModel1, g_iOffset_ViewModelSequence, -1, _, true);
        SDKCall(hSDKCallEntityUpdateTransmitState, viewModel2);
        
        // Sets the model entity for the main weapon
        SetEntityModel(weaponIndex, sModel);

        // If the sequence for the weapon didn't build yet
        if(WeaponsGetSequenceCount(iD) == -1)
        {
            // Gets the sequence amount from a weapon entity
            int nSequenceCount = Animating_GetSequenceCount(weaponIndex);

            // Validate count
            if(nSequenceCount)
            {
                // Initialize the sequence array
                int iSequences[WeaponsSequencesMax];

                // Validate amount
                if(nSequenceCount < WeaponsSequencesMax)
                {
                    // Build the sequence array
                    WeaponHDRBuildSwapSequenceArray(iSequences, nSequenceCount, weaponIndex);
                    
                    // Update the sequence array
                    WeaponsSetSequenceCount(iD, nSequenceCount);
                    WeaponsSetSequenceSwap(iD, iSequences, WeaponsSequencesMax);
                }
                else
                {
                    // Unexpected error, log it
                    LogEvent(false, LogType_Error, LOG_CORE_EVENTS, LogModule_Weapons, "Weapons HDR", "View model \"%s\" is having too many sequences! (Max %i, is %i) - Increase value of WeaponsSequencesMax in plugin", sModel, WeaponsSequencesMax, nSequenceCount);
                }
            }
            else
            {
                // Remove swapped weapon
                WeaponsClearSequenceSwap(iD);
                
                // Unexpected error, log it
                LogEvent(false, LogType_Error, LOG_CORE_EVENTS, LogModule_Weapons, "Weapons HDR", "Failed to get sequence count for weapon using model \"%s\" - Animations may not work as expected", sModel);
            }
        }

        // Sets the model/body/skin index for view model
        SetEntData(viewModel2, g_iOffset_EntityModelIndex, iModel, _, true);
        SetEntData(viewModel2, g_iOffset_WeaponBody, WeaponsGetModelBody(iD), _, true);
        SetEntData(viewModel2, g_iOffset_WeaponSkin, WeaponsGetModelSkin(iD), _, true);
        
        //  Update the animation interval delay for second view model 
        SetEntDataFloat(viewModel2, g_iOffset_ViewModelPlaybackRate, GetEntDataFloat(viewModel1, g_iOffset_ViewModelPlaybackRate), true);

        // Create a toggle model
        WeaponHDRToggleViewModel(clientIndex, viewModel2, iD);
        
        // Resets the sequence parity
        gClientData[clientIndex][Client_LastSequenceParity] = -1;
    }
    else
    {
        // Clear the custom id cache
        gClientData[clientIndex][Client_CustomWeapon] = 0;
    }

    // If world model exist, then apply it
    if(WeaponsGetModelWorldID(iD))
    {
        WeaponHDRSetPlayerWorldModel(weaponIndex, WeaponsGetModelWorldID(iD), WeaponsGetModelBody(iD), WeaponsGetModelSkin(iD));
    }
    // If it don't exist, then hide it
    else
    {
        // Verify that the client is zombie
        if(gClientData[clientIndex][Client_Zombie])
        {
            // Validate a knife
            if(WeaponsValidateKnife(weaponIndex)) WeaponHDRSetPlayerWorldModel(weaponIndex);
        }
    }
}

/**
 * Hook: PostThinkPost
 * Player hold any weapon.
 *
 * @param clientIndex       The client index.
 **/
public void WeaponSDKOnAnimationFix(int clientIndex) 
{
    // Validate client
    if(!IsPlayerExist(clientIndex, false))
    {
        return;
    }

    // Set current addons
    WeaponAttachSetAddons(clientIndex); /// Back weapon models
    
    // Validate weapon
    if(!gClientData[clientIndex][Client_CustomWeapon]) /// Optimization for frame check
    {
        return;
    }

    // Gets the entity index from the reference
    int viewModel1 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][0]);
    int viewModel2 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][1]);

    // Validate view models
    if(viewModel1 == INVALID_ENT_REFERENCE || viewModel2 == INVALID_ENT_REFERENCE)
    {
        return;
    }

    // Gets the sequence number and it draw animation sequence
    int nSequence = GetEntData(viewModel1, g_iOffset_ViewModelSequence);
    int drawSequence = gClientData[clientIndex][Client_DrawSequence];
    
    // Validate sequence
    if(nSequence == -1)
    {
        nSequence = drawSequence; /// Play the draw animation
    }
    
    // Gets the sequence parity index
    int sequenceParity = GetEntData(viewModel1, g_iOffset_NewSequenceParity);

    // Sequence has not changed since last think
    if(nSequence == gClientData[clientIndex][Client_LastSequence])
    {
        // Skip on weapon switch
        if(gClientData[clientIndex][Client_LastSequenceParity] != -1)
        {
            // Skip if sequence hasn't finished
            if(sequenceParity == gClientData[clientIndex][Client_LastSequenceParity])
            {
                return;
            }

            // Gets the weapon id from the reference
            int iD = gClientData[clientIndex][Client_WeaponIndex]; /// Only view model identification
            int swapSequence = WeaponsGetSequenceSwap(iD, nSequence);
            
            // Change to swap sequence, if present
            if(swapSequence != -1)
            {
                // Play the swaped sequence
                SetEntData(viewModel1, g_iOffset_ViewModelSequence, swapSequence, _, true);
                SetEntData(viewModel2, g_iOffset_ViewModelSequence, swapSequence, _, true);

                // Update the sequence for next check
                gClientData[clientIndex][Client_LastSequence] = swapSequence;
            }
            else
            {
                // Create a toggle model
                WeaponHDRToggleViewModel(clientIndex, viewModel2, iD);
            }
        }
    }
    else
    {
        // Validate sequence
        if(drawSequence != -1 && nSequence != drawSequence)
        {
            SDKCall(hSDKCallEntityUpdateTransmitState, viewModel1); /// Update!
            gClientData[clientIndex][Client_DrawSequence] = -1;
        }
        
        // Sets the new sequence
        SetEntData(viewModel2, g_iOffset_ViewModelSequence, nSequence, _, true);
        gClientData[clientIndex][Client_LastSequence] = nSequence;
    }
    
    // Update the sequence parity for next check
    gClientData[clientIndex][Client_LastSequenceParity] = sequenceParity;
}

/**
 * Event: WeaponOnFire
 * Weapon has been fired.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
void WeaponSDKOnFire(int clientIndex, int weaponIndex) 
{ 
    // Validate custom index
    int iD = WeaponsGetCustomID(weaponIndex);
    if(iD != INVALID_ENT_REFERENCE)    
    {
        // Returns the game time based on the game tick
        float flCurrentTime = GetGameTime();

        // If view model exist, then apply muzzle smoke
        if(WeaponsGetModelViewID(iD))
        {
            // Initialize variables
            static float flHeatDelay[MAXPLAYERS+1]; static float flSmoke[MAXPLAYERS+1];

            // Gets the entity index from the reference
            int viewModel2 = EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][1]);

            // Validate secondary view model
            if(viewModel2 == INVALID_ENT_REFERENCE)
            {
                return;
            }

            // If weapon without any type of ammo, then stop
            if(GetEntData(weaponIndex, g_iOffset_WeaponAmmoType) == -1)
            {
                return;
            }

            // Calculate the expected heat amount
            float flHeat = ((flCurrentTime - GetEntDataFloat(weaponIndex, g_iOffset_LastShotTime)) * -0.5) + flHeatDelay[clientIndex];

            // This value is set specifically for each weapon
            flHeat += WeaponsGetModelHeat(iD);
            
            // Reset the heat
            if(flHeat < 0.0) flHeat = 0.0;

            // Validate heat
            if(flHeat > 1.0)
            {
                // Validate delay
                if(flCurrentTime - flSmoke[clientIndex] > 1.0)
                {
                    // Create a muzzle smoke
                    VEffectSpawnMuzzleSmoke(clientIndex, viewModel2);
                    flSmoke[clientIndex] = flCurrentTime;
                }
                
                // Resets then
                flHeat = 0.0;
            }

            // Update the heat delay
            flHeatDelay[clientIndex] = flHeat;
        }
        
        // If custom fire speed exist, then apply it
        float flSpeed = WeaponsGetSpeed(iD);
        if(flSpeed)
        {
            // Sets the next attack time
            SetEntDataFloat(clientIndex, g_iOffset_PlayerAttack, flCurrentTime + flSpeed, true);
        }
    }
    
    // Validate a non-knife
    if(!WeaponsValidateKnife(weaponIndex) && !WeaponsValidateTaser(weaponIndex)) 
    {
        // Validate current ammunition mode
        switch(gClientData[clientIndex][Client_Survivor] ? gCvarList[CVAR_SURVIVOR_INF_AMMUNITION].IntValue : gCvarList[CVAR_HUMAN_INF_AMMUNITION].IntValue)
        {
            case 0 : return;
            case 1 : { SetEntData(weaponIndex, g_iOffset_WeaponReserve1, GetEntData(weaponIndex, g_iOffset_WeaponReserve2), _, true); }
            case 2 : { SetEntData(weaponIndex, g_iOffset_WeaponClip1, GetEntData(weaponIndex, g_iOffset_WeaponClip1) + 1, _, true); } 
        }
    }
}

/**
 * Event: WeaponOnFire
 * Weapon has been shoot.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
Action WeaponSDKOnShoot(int clientIndex, int weaponIndex) 
{ 
    #pragma unused clientIndex
    
    // Validate custom index
    int iD = WeaponsGetCustomID(weaponIndex);
    if(iD != INVALID_ENT_REFERENCE)    
    {
        // Validate broadcast
        return SoundsInputEmit(weaponIndex, SNDCHAN_WEAPON, WeaponsGetSoundID(iD)) ? Plugin_Stop : Plugin_Continue;
    }
    
    // Allow broadcast
    return Plugin_Continue;
}

/**
 * Event: WeaponOnHostage
 * Weapon has been switch by hostage.
 *
 * @param clientIndex       The client index.
 * @param weaponIndex       The weapon index.
 **/
void WeaponSDKOnHostage(int clientIndex) 
{
    // Prevent the view model from being removed
    WeaponHDRSetPlayerViewModel(clientIndex, 1, INVALID_ENT_REFERENCE);

    // Apply fake hostage follow hook on the next frame
    RequestFrame(view_as<RequestFrameCallback>(WeaponSDKOnFakeHostage), GetClientUserId(clientIndex));
}
    
/**
 * FakeEvent: WeaponOnFakeHostage
 *
 * @param userID            The user id.
 **/
public void WeaponSDKOnFakeHostage(int userID) 
{
    // Gets the client index from the user ID
    int clientIndex = GetClientOfUserId(userID);

    // Validate client
    if(clientIndex)
    {
        // Gets the weapon id from the reference
        int iD = gClientData[clientIndex][Client_WeaponIndex]; /// Only view model identification

        // Validate id
        if(iD != INVALID_ENT_REFERENCE)
        {
            // Gets the second view model
            int viewModel2 = WeaponHDRGetPlayerViewModel(clientIndex, 1);

            // Remove the view model created by the game
            if(IsValidEdict(viewModel2))
            {
                AcceptEntityInput(viewModel2, "Kill");
            }

            // Resets the view model
            WeaponHDRSetPlayerViewModel(clientIndex, 1, EntRefToEntIndex(gClientData[clientIndex][Client_ViewModels][1]));
        }
    }
}

/**
 * Callback for command listener to buy ammunition.
 *
 * @param entityIndex       The client index.
 * @param commandMsg        Command name, lower case. To get name as typed, use GetCmdArg() and specify argument 0.
 * @param iArguments        Argument count.
 **/
public Action WeaponSDKOnAmmunition(int clientIndex, const char[] commandMsg, int iArguments)
{
    // Validate client
    if(IsPlayerExist(clientIndex))
    {
        // Validate current ammunition mode
        switch(gClientData[clientIndex][Client_Survivor] ? gCvarList[CVAR_SURVIVOR_INF_AMMUNITION].IntValue : gCvarList[CVAR_HUMAN_INF_AMMUNITION].IntValue)
        {
            case 0 : { /* empty statement */ }
            default: return Plugin_Continue;
        }

        // Validate ammunition cost
        int iCost = gClientData[clientIndex][Client_Survivor] ? gCvarList[CVAR_SURVIVOR_PRICE_AMMUNITION].IntValue : gCvarList[CVAR_HUMAN_PRICE_AMMUNITION].IntValue;
        if(!iCost || gClientData[clientIndex][Client_AmmoPacks] < iCost)
        {
            return Plugin_Continue;
        }

        // Gets the active weapon index from the client
        int weaponIndex = GetEntDataEnt2(clientIndex, g_iOffset_PlayerActiveWeapon);

        // Validate weapon
        if(IsValidEdict(weaponIndex))
        {
            // If weapon without any type of ammo, then stop
            if(GetEntData(weaponIndex, g_iOffset_WeaponAmmoType) == -1)
            {
                return Plugin_Continue;
            }
    
            // Gets current/max reverse ammo
            int iAmmo = GetEntData(weaponIndex, g_iOffset_WeaponReserve1);
            int iMaxAmmo = SDKCall(hSDKCallGetReserveAmmoMax, weaponIndex);
            
            // Reset ammomax for standart weapons
            if(!iMaxAmmo) iMaxAmmo = GetEntData(weaponIndex, g_iOffset_WeaponReserve2); /// Bug fix for standart weapons
            
            // Validate amount
            if(iAmmo < iMaxAmmo)
            {
                // Generate amount
                iAmmo += SDKCall(hSDKCallGetMaxClip1, weaponIndex);

                // Gives ammo of a certain type to a weapon
                SetEntData(weaponIndex, g_iOffset_WeaponReserve1, (iAmmo <= iMaxAmmo) ? iAmmo : iMaxAmmo, _, true);

                // Remove ammopacks
                AccountSetClientCash(clientIndex, gClientData[clientIndex][Client_AmmoPacks] - iCost);

                // Forward event to modules
                SoundsOnClientAmmunition(clientIndex);
            }
        }
    }

    // Allow commands
    return Plugin_Continue;
}

#if defined USE_DHOOKS
/**
 * DHook: Sets a weapon clip when its spawned, picked, dropped or reloaded.
 * int CBaseCombatWeapon::GetMaxClip1(void *)
 *
 * @param weaponIndex       The weapon index.
 * @param hReturn           Handle to return structure.
 **/
public MRESReturn WeaponDHookOnGetMaxClip1(int weaponIndex, Handle hReturn)
{
    // Validate custom index
    int iD = WeaponsGetCustomID(weaponIndex);
    if(iD != INVALID_ENT_REFERENCE)
    {
        // Gets the weapon clip
        int iClip = WeaponsGetClip(iD);
        if(iClip)
        {
            DHookSetReturn(hReturn, iClip);
            return MRES_Override;
        }
    }

    // Skip the hook
    return MRES_Ignored;
}

/**
 * DHook: Sets a weapon reserved ammunition when its spawned, picked, dropped or reloaded. 
 * int CBaseCombatWeapon::GetReserveAmmoMax(AmmoPosition_t)
 *
 * @param weaponIndex       The weapon index.
 * @param hReturn           Handle to return structure.
 **/
public MRESReturn WeaponDHookOnGetReverseMax(int weaponIndex, Handle hReturn)
{
    // Validate custom index
    int iD = WeaponsGetCustomID(weaponIndex);
    if(iD != INVALID_ENT_REFERENCE)
    {
        // Gets the weapon ammo
        int iAmmo = WeaponsGetAmmo(iD);
        if(iAmmo)
        {
            DHookSetReturn(hReturn, iAmmo);
            return MRES_Override;
        }
    }
    
    // Skip the hook
    return MRES_Ignored;
}
#endif
