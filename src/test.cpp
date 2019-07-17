#include <iostream>


#include "al/app/al_App.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#include "curl/curl.h"

using namespace al;

struct MyApp : App {

  ParameterBool mute {"Mute"};
//  Trigger volumeUp {"+"};
  float volume = 0.0;
//  Trigger volumeDown {"+"};

  std::vector<std::pair<uint16_t, std::string>> deviceAddresses = {{3001,"motu0.1g"}, {3001,"motu1.1g"}, {3001,"motu2.1g"}};

  void onCreate() override {
    al::imguiInit();

    mute.registerChangeCallback([&](float value) {
      setMute(value == 1.0f);
    });
  }

  void onAnimate(double dt) {

    al::imguiBeginFrame();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(width(), height()));
    ImGui::Begin("MOTU Control", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize );

    ImGui::SetWindowFontScale(4.0);

    ParameterGUI::draw(&mute);

    ImGui::End();
    al::imguiEndFrame();
  }

  void onDraw(Graphics& g) override {
    g.clear(0, 0, 0);

    al::imguiDraw();
  }

  void onExit() override {
      ParameterGUI::cleanup();
  }

  struct MemoryStruct {
    char *memory;
    size_t size;
  };

  void setMute(bool isMute) {
    size_t numChannels = 12;
    for (auto device: deviceAddresses) {
      osc::Send sender(device.first, device.second.c_str());
      for (size_t i = 0; i < numChannels; i++) {
        std::string address = "/mix/chan/" + std::to_string(i) + "/matrix/mute";
//        sender.send(address, isMute ? 1.0f : 0.0f);
      }
//        address = "http://" + device.second + ":" + std::to_string(device.first) + address;
//        httpGet(address);
    }
  }

  void setLevel(float level) {
    size_t numChannels = 12;
    for (auto device: deviceAddresses) {
      osc::Send sender(device.first, device.second.c_str());
      for (size_t i = 0; i < numChannels; i++) {
        std::string address = "/mix/chan/" + std::to_string(i) + "/matrix/fader";
        sender.send(address, level);
      }
//        address = "http://" + device.second + ":" + std::to_string(device.first) + address;
//        httpGet(address);
    }
  }

  // HTTP requests
  void httpGet(std::string address) {
      CURL *curl_handle;
      CURLcode res;

      struct MemoryStruct chunk;

      chunk.memory = (char *) std::malloc(1);  /* will be grown as needed by the realloc above */
      chunk.size = 0;    /* no data at this point */

      curl_global_init(CURL_GLOBAL_ALL);

      curl_handle = curl_easy_init();
      curl_easy_setopt(curl_handle, CURLOPT_URL, address.c_str());
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
      curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

      res = curl_easy_perform(curl_handle);
  }

  static size_t
  WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
  {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == nullptr) {
      /* out of memory! */
      printf("not enough memory (realloc returned NULL)\n");
      return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
  }

};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.start();
}
