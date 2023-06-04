/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_STEAMAPI_H_INCL
#define LIBTAS_STEAMAPI_H_INCL

#include "../hook.h"
#include "isteamclient/isteamclient.h"
#include "isteamcontroller.h"
#include "isteamuser.h"
#include "isteamuserstats.h"
#include "isteamutils.h"
#include "isteamremotestorage/isteamremotestorage.h"
#include "isteamapps.h"
#include "isteamfriends.h"
#include "isteamscreenshots.h"
#include "isteamugc.h"
#include "isteammatchmaking.h"
#include "isteamhttp.h"
#include "isteaminput.h"
#include "isteamnetworking.h"
#include "isteamnetworkingutils.h"
#include "isteamnetworkingsockets.h"
#include "isteamnetworkingmessages.h"
#include "CCallback.h"
#include "CCallbackManager.h"

namespace libtas {

OVERRIDE void SteamAPI_Shutdown();

// checks if a local Steam client is running
OVERRIDE bool SteamAPI_IsSteamRunning();

// Detects if your executable was launched through the Steam client, and restarts your game through
// the client if necessary. The Steam client will be started if it is not running.
//
// Returns: true if your executable was NOT launched through the Steam client. This function will
//          then start your application through the client. Your current process should exit.
//
//          false if your executable was started through the Steam client or a steam_appid.txt file
//          is present in your game's directory (for development). Your current process should continue.
//
// NOTE: This function should be used only if you are using CEG or not using Steam's DRM. Once applied
//       to your executable, Steam's DRM will handle restarting through Steam if necessary.
OVERRIDE bool SteamAPI_RestartAppIfNecessary( unsigned int unOwnAppID );

OVERRIDE bool SteamAPI_Init();

OVERRIDE bool SteamAPI_InitSafe();

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	steam callback helper functions
//
//	The following classes/macros are used to be able to easily multiplex callbacks
//	from the Steam API into various objects in the app in a thread-safe manner
//
//	These functors are triggered via the SteamAPI_RunCallbacks() function, mapping the callback
//  to as many functions/objects as are registered to it
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

OVERRIDE void SteamAPI_RunCallbacks();

// functions used by the utility CCallback objects to receive callbacks
OVERRIDE void SteamAPI_RegisterCallback( CCallbackBase *pCallback, enum steam_callback_type iCallback );
OVERRIDE void SteamAPI_UnregisterCallback( CCallbackBase *pCallback );

// Internal functions used by the utility CCallResult objects to receive async call results
OVERRIDE void SteamAPI_RegisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall );
OVERRIDE void SteamAPI_UnregisterCallResult( CCallbackBase *pCallback, SteamAPICall_t hAPICall );

/// Inform the API that you wish to use manual event dispatch.  This must be called after SteamAPI_Init, but before
/// you use any of the other manual dispatch functions below.
OVERRIDE void SteamAPI_ManualDispatch_Init();

/// Perform certain periodic actions that need to be performed.
OVERRIDE void SteamAPI_ManualDispatch_RunFrame( HSteamPipe hSteamPipe );

/// Fetch the next pending callback on the given pipe, if any.  If a callback is available, true is returned
/// and the structure is populated.  In this case, you MUST call SteamAPI_ManualDispatch_FreeLastCallback
/// (after dispatching the callback) before calling SteamAPI_ManualDispatch_GetNextCallback again.
OVERRIDE bool SteamAPI_ManualDispatch_GetNextCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg );

/// You must call this after dispatching the callback, if SteamAPI_ManualDispatch_GetNextCallback returns true.
OVERRIDE void SteamAPI_ManualDispatch_FreeLastCallback( HSteamPipe hSteamPipe );

/// Return the call result for the specified call on the specified pipe.  You really should
/// only call this in a handler for SteamAPICallCompleted_t callback.
OVERRIDE bool SteamAPI_ManualDispatch_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed );

OVERRIDE ISteamController *SteamController();
OVERRIDE ISteamUserStats *SteamUserStats();
OVERRIDE ISteamUser *SteamUser();
OVERRIDE ISteamUtils *SteamUtils();
OVERRIDE ISteamApps *SteamApps();
OVERRIDE ISteamFriends *SteamFriends();
OVERRIDE ISteamScreenshots *SteamScreenshots();
OVERRIDE ISteamUGC *SteamUGC();

typedef void ISteamAppList;
typedef void ISteamMusic;
typedef void ISteamMusicRemote;
typedef void ISteamHTMLSurface;
typedef void ISteamInventory;
typedef void ISteamVideo;
typedef void ISteamParentalSettings;

OVERRIDE ISteamMatchmaking *SteamMatchmaking();
OVERRIDE ISteamNetworking *SteamNetworking();
OVERRIDE ISteamMatchmakingServers *SteamMatchmakingServers();
OVERRIDE ISteamHTTP *SteamHTTP();
OVERRIDE ISteamInput *SteamInput();
OVERRIDE ISteamAppList *SteamAppList();
OVERRIDE ISteamMusic *SteamMusic();
OVERRIDE ISteamMusicRemote *SteamMusicRemote();
OVERRIDE ISteamHTMLSurface *SteamHTMLSurface();
OVERRIDE ISteamInventory *SteamInventory();
OVERRIDE ISteamVideo *SteamVideo();
OVERRIDE ISteamParentalSettings *SteamParentalSettings();
OVERRIDE ISteamNetworkingUtils *SteamNetworkingUtils();
OVERRIDE ISteamNetworkingSockets *SteamNetworkingSockets();
OVERRIDE ISteamNetworkingMessages *SteamNetworkingMessages();

}

#endif
