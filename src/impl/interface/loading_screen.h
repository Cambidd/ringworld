#ifndef RINGWORLD__INTERFACE__UI_LOADIN_SCREEN_H
#define RINGWORLD__INTERFACE__UI_LOADIN_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

enum UILoadingScreenState {
    LOADING_SCREEN_STATE_OFF,
    LOADING_SCREEN_STATE_LOCAL,
    LOADING_SCREEN_STATE_SERVER,
    LOADING_SCREEN_STATE_RESOLVING,
    LOADING_SCREEN_STATE_NEGOTIATING,
    LOADING_SCREEN_STATE_CONNECTING,
    LOADING_SCREEN_STATE_RETRYING,
    LOADING_SCREEN_STATE_CONNECTED,
    LOADING_SCREEN_STATE_LOADING_MP_MAP,
    LOADING_SCREEN_STATE_WAITING_FOR_NEW_GAME,
    NUM_OF_LOADING_SCREEN_STATES,
};

#ifdef __cplusplus
}
#endif

#endif
