#pragma once

#include "Json.h"

// Test-fixture data only: fabricates a plausible contact list as a JSON
// array of objects (name, phone, company, dob), for exercising the JSON
// library. Not part of the shipped CRUD app's data model.
JsonValue makeRandomContacts(int count = 10);
