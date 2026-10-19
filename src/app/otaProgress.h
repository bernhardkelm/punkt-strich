#pragma once

class StateMachine;

/**
 * Starts the OTA service with callbacks that show the progress spiral while an update runs.
 * @param {const char*} name - hostname the OTA service announces
 * @param {StateMachine&} state - state machine the spiral runs on, kept for the callbacks
 * @returns {void}
 */
void beginOta(const char* name, StateMachine& state);
