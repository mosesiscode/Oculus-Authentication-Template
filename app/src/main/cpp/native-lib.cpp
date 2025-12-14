#include <jni.h>
#include "BNM/Loading.hpp"
#include "BNM/Class.hpp"
#include "BNMResolve.hpp"
#include <httplib.h>
#include <json.hpp>

bool g_isAuthenticated = false;
std::string g_userOculusID = "";

bool authenticateWithOculus() {
    // Put your FRL or OC token here
    const char* TOKEN = "FRL|your_token_here"; // Or OC%your_oc_token

    // Check for FRL token
    bool isFRL = (strlen(TOKEN) > 10 && strncmp(TOKEN, "FRL", 3) == 0);

    // Check for OC token
    bool isOC = (strlen(TOKEN) > 10 && strncmp(TOKEN, "OC", 2) == 0);

    if (isFRL || isOC) {
        BNM_LOG_INFO("Authenticated with %s token", isFRL ? "FRL" : "OC");
        return true;
    } else {
        BNM_LOG_ERR("Invalid token format - mod disabled");
        return false;
    }
}

void (*Awake)(void*);
void new_Awake(void* instance) {
    Awake(instance);

    // Only execute if authenticated
    if (g_isAuthenticated) {
        nlohmann::json body{};
        body["content"] = "Authenticated user loaded mod!";

        httplib::SSLClient cli("discord.com", 443);
        if (auto req = cli.Post("api/webhooks/1/1", body.dump(), "application/json")) {
            BNM_LOG_INFO("Made request");
        }
    } else {
        BNM_LOG_WARN("Not authenticated - webhook disabled");
    }
}

void (*LateUpdate)(void*);
void new_LateUpdate(void* instance) {
    // Only execute mod code if authenticated
    if (g_isAuthenticated) {
        // Your mod code here

        // Example: Do something in LateUpdate
    }

    LateUpdate(instance);
}

void OnLoaded() {
    BNM_LOG_INFO("Loaded Successfully");

    BNM_LOG_INFO("Starting Oculus authentication...");
    g_isAuthenticated = authenticateWithOculus();

    if (!g_isAuthenticated) {
        BNM_LOG_ERR("Authentication failed - mod features will be disabled");
        // Optionally return here to prevent hooks from being installed
        // return;
    }

    InvokeHook(Class(BNM_OBFUSCATE("GorillaLocomotion"), BNM_OBFUSCATE("Player"), Image(
            BNM_OBFUSCATE("Assembly-CSharp.dll"))).GetMethod(BNM_OBFUSCATE("LateUpdate")), new_LateUpdate, LateUpdate);

    InvokeHook(Class(BNM_OBFUSCATE("GorillaLocomotion"), BNM_OBFUSCATE("Player"), Image(
            BNM_OBFUSCATE("Assembly-CSharp.dll"))).GetMethod(BNM_OBFUSCATE("Awake")), new_Awake, Awake);

    BNM_LOG_INFO("All hooks installed");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    BNM::Loading::AddOnLoadedEvent(OnLoaded);

    BNM::Loading::TryLoadByJNI(env);

    return JNI_VERSION_1_6;
}