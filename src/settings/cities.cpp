#include "settings/cities.h"

#include <cstring>

namespace {

constexpr const char* kEurope = "Europe";
constexpr const char* kAmerica = "America";
constexpr const char* kAsia = "Asia";
constexpr const char* kPacific = "Australia and Pacific";
constexpr const char* kAfrica = "Africa";
constexpr const char* kOther = "Other and UTC";
constexpr size_t kDefaultIndex = 6;

constexpr City kCities[] = {
    {"Europe/London", "London", kEurope, "GMT0BST,M3.5.0/1,M10.5.0", 51.51f, -0.13f},
    {"Europe/Dublin", "Dublin", kEurope, "IST-1GMT0,M10.5.0,M3.5.0/1", 53.35f, -6.26f},
    {"Europe/Lisbon", "Lisbon", kEurope, "WET0WEST,M3.5.0/1,M10.5.0", 38.72f, -9.14f},
    {"Europe/Madrid", "Madrid", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 40.42f, -3.70f},
    {"Europe/Paris", "Paris", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 48.86f, 2.35f},
    {"Europe/Amsterdam", "Amsterdam", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 52.37f, 4.90f},
    {"Europe/Berlin", "Berlin", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 52.52f, 13.41f},
    {"Europe/Zurich", "Zurich", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 47.38f, 8.54f},
    {"Europe/Rome", "Rome", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 41.90f, 12.50f},
    {"Europe/Stockholm", "Stockholm", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 59.33f, 18.07f},
    {"Europe/Helsinki", "Helsinki", kEurope, "EET-2EEST,M3.5.0/3,M10.5.0/4", 60.17f, 24.94f},
    {"Europe/Warsaw", "Warsaw", kEurope, "CET-1CEST,M3.5.0,M10.5.0/3", 52.23f, 21.01f},
    {"Europe/Athens", "Athens", kEurope, "EET-2EEST,M3.5.0/3,M10.5.0/4", 37.98f, 23.73f},
    {"Europe/Bucharest", "Bucharest", kEurope, "EET-2EEST,M3.5.0/3,M10.5.0/4", 44.43f, 26.10f},
    {"Europe/Istanbul", "Istanbul", kEurope, "<+03>-3", 41.01f, 28.98f},
    {"Europe/Kyiv", "Kyiv", kEurope, "EET-2EEST,M3.5.0/3,M10.5.0/4", 50.45f, 30.52f},
    {"Europe/Moscow", "Moscow", kEurope, "MSK-3", 55.76f, 37.62f},
    {"America/Halifax", "Halifax", kAmerica, "AST4ADT,M3.2.0,M11.1.0", 44.65f, -63.57f},
    {"America/New_York", "New York", kAmerica, "EST5EDT,M3.2.0,M11.1.0", 40.71f, -74.01f},
    {"America/Toronto", "Toronto", kAmerica, "EST5EDT,M3.2.0,M11.1.0", 43.65f, -79.38f},
    {"America/Chicago", "Chicago", kAmerica, "CST6CDT,M3.2.0,M11.1.0", 41.88f, -87.63f},
    {"America/Mexico_City", "Mexico City", kAmerica, "CST6", 19.43f, -99.13f},
    {"America/Denver", "Denver", kAmerica, "MST7MDT,M3.2.0,M11.1.0", 39.74f, -104.99f},
    {"America/Phoenix", "Phoenix", kAmerica, "MST7", 33.45f, -112.07f},
    {"America/Los_Angeles", "Los Angeles", kAmerica, "PST8PDT,M3.2.0,M11.1.0", 34.05f, -118.24f},
    {"America/Vancouver", "Vancouver", kAmerica, "PST8PDT,M3.2.0,M11.1.0", 49.28f, -123.12f},
    {"America/Anchorage", "Anchorage", kAmerica, "AKST9AKDT,M3.2.0,M11.1.0", 61.22f, -149.90f},
    {"America/Bogota", "Bogota", kAmerica, "<-05>5", 4.71f, -74.07f},
    {"America/Santiago", "Santiago", kAmerica, "<-04>4<-03>,M9.1.6/24,M4.1.6/24", -33.45f, -70.67f},
    {"America/Argentina/Buenos_Aires", "Buenos Aires", kAmerica, "<-03>3", -34.60f, -58.38f},
    {"America/Sao_Paulo", "Sao Paulo", kAmerica, "<-03>3", -23.55f, -46.63f},
    {"Asia/Jerusalem", "Jerusalem", kAsia, "IST-2IDT,M3.4.4/26,M10.5.0", 31.77f, 35.21f},
    {"Asia/Riyadh", "Riyadh", kAsia, "<+03>-3", 24.71f, 46.68f},
    {"Asia/Dubai", "Dubai", kAsia, "<+04>-4", 25.20f, 55.27f},
    {"Asia/Tehran", "Tehran", kAsia, "<+0330>-3:30", 35.69f, 51.39f},
    {"Asia/Karachi", "Karachi", kAsia, "PKT-5", 24.86f, 67.01f},
    {"Asia/Kolkata", "Kolkata", kAsia, "IST-5:30", 22.57f, 88.36f},
    {"Asia/Dhaka", "Dhaka", kAsia, "<+06>-6", 23.81f, 90.41f},
    {"Asia/Bangkok", "Bangkok", kAsia, "<+07>-7", 13.76f, 100.50f},
    {"Asia/Jakarta", "Jakarta", kAsia, "WIB-7", -6.21f, 106.85f},
    {"Asia/Ho_Chi_Minh", "Ho Chi Minh City", kAsia, "<+07>-7", 10.82f, 106.63f},
    {"Asia/Singapore", "Singapore", kAsia, "<+08>-8", 1.35f, 103.82f},
    {"Asia/Hong_Kong", "Hong Kong", kAsia, "HKT-8", 22.32f, 114.17f},
    {"Asia/Shanghai", "Shanghai", kAsia, "CST-8", 31.23f, 121.47f},
    {"Asia/Manila", "Manila", kAsia, "PST-8", 14.60f, 120.98f},
    {"Asia/Seoul", "Seoul", kAsia, "KST-9", 37.57f, 126.98f},
    {"Asia/Tokyo", "Tokyo", kAsia, "JST-9", 35.68f, 139.69f},
    {"Australia/Perth", "Perth", kPacific, "AWST-8", -31.95f, 115.86f},
    {"Australia/Darwin", "Darwin", kPacific, "ACST-9:30", -12.46f, 130.84f},
    {"Australia/Brisbane", "Brisbane", kPacific, "AEST-10", -27.47f, 153.03f},
    {"Australia/Adelaide", "Adelaide", kPacific, "ACST-9:30ACDT,M10.1.0,M4.1.0/3", -34.93f, 138.60f},
    {"Australia/Sydney", "Sydney", kPacific, "AEST-10AEDT,M10.1.0,M4.1.0/3", -33.87f, 151.21f},
    {"Pacific/Auckland", "Auckland", kPacific, "NZST-12NZDT,M9.5.0,M4.1.0/3", -36.85f, 174.76f},
    {"Pacific/Fiji", "Fiji", kPacific, "<+12>-12", -18.14f, 178.44f},
    {"Pacific/Honolulu", "Honolulu", kPacific, "HST10", 21.31f, -157.86f},
    {"Africa/Casablanca", "Casablanca", kAfrica, "<+01>-1", 33.57f, -7.59f},
    {"Africa/Lagos", "Lagos", kAfrica, "WAT-1", 6.52f, 3.38f},
    {"Africa/Accra", "Accra", kAfrica, "GMT0", 5.60f, -0.19f},
    {"Africa/Cairo", "Cairo", kAfrica, "EET-2EEST,M4.5.5/0,M10.5.4/24", 30.04f, 31.24f},
    {"Africa/Nairobi", "Nairobi", kAfrica, "EAT-3", -1.29f, 36.82f},
    {"Africa/Johannesburg", "Johannesburg", kAfrica, "SAST-2", -26.20f, 28.05f},
    {"UTC", "UTC", kOther, "UTC0", 0.0f, 0.0f},
    {"Atlantic/Reykjavik", "Reykjavik", kOther, "GMT0", 64.15f, -21.94f},
    {"Atlantic/Azores", "Azores", kOther, "<-01>1<+00>,M3.5.0/0,M10.5.0/1", 37.74f, -25.67f},
};

constexpr size_t kCount = sizeof(kCities) / sizeof(kCities[0]);

}

const City* findCity(const char* name) {
  if (name == nullptr) {
    return nullptr;
  }

  for (size_t i = 0; i < kCount; i++) {
    if (strcmp(name, kCities[i].name) == 0) {
      return &kCities[i];
    }
  }

  return nullptr;
}

size_t cityCount() {
  return kCount;
}

const City& cityAt(size_t index) {
  return kCities[index < kCount ? index : kCount - 1];
}

const City& defaultCity() {
  return kCities[kDefaultIndex];
}
