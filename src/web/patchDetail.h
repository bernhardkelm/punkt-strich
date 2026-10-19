#pragma once

#include <WString.h>

struct SettingsPatch;

/**
 * Lists the fields a patch carries for the log line.
 * @param {const SettingsPatch&} patch - validated fields
 * @returns {String} - space separated assignments, empty when the body carried no known field
 */
String patchDetail(const SettingsPatch& patch);
