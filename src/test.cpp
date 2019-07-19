#include <iostream>


#include "al/app/al_App.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#define USE_CURL

#ifdef USE_CURL
#include "curl/curl.h"
#endif

using namespace al;

struct AVBDevice {
  std::string host;
  uint16_t oscPort;
  int bankIndex;
  size_t numChannels = 64;
};


struct MyApp : App {

  ParameterBool mute {"Mute"};
  Trigger volumeUp {"Up"};
  float volume = -12.0;
  Trigger volumeDown {"Down"};

  std::vector<AVBDevice> devices = {
    {"motu00.1g", 9998, 0, 16},
    {"motu01.1g", 9998, 0, 24},
    {"motu02.1g", 9998, 0, 24},
  };

  void onInit() override {
    decorated(false);
  }
  void onCreate() override {
    al::imguiInit();

    mute.registerChangeCallback([&](float value) {
      setMute(value == 1.0f);
    });

    volumeUp.registerChangeCallback([&](float value) {
      volume++;
      setVolume();
    });
    volumeDown.registerChangeCallback([&](float value) {
      volume--;
      setVolume();
    });
  }

  void onAnimate(double dt) {

    al::imguiBeginFrame();
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(width(), height()));
    ImGui::Begin("MOTU Control", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize );

    ImGui::SetWindowFontScale(5.0);

    ImGui::PushItemWidth(width());
    ParameterGUI::draw(&mute);
    if (mute != 1.0f) {
      ParameterGUI::draw(&volumeDown);
      ImGui::SameLine();
      ImGui::Text("%i", int(volume));
      ImGui::SameLine();
      ParameterGUI::draw(&volumeUp);
    }

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

#ifdef USE_CURL

  struct MemoryStruct {
    char *memory;
    size_t size;
  };
#endif

  void setMute(bool isMute) {
    if (isMute) {
      for (auto device: devices) {
        for (size_t i = 0; i < device.numChannels; i++) {
#ifdef USE_CURL
          std::string address = "http://" + device.host + "/datastore/ext/";
          address += "obank/" + std::to_string(device.bankIndex) + "/";
          address += "ch/" + std::to_string(i) + "/trim";
          std::string data = "json={\"value\":-24}";
          httpPost(address, data);
#else
          osc::Send sender(device.oscPort.first, device.host.c_str());
          std::string address = "/mix/chan/" + std::to_string(i) + "/matrix/mute";
          sender.send(address, isMute ? 1.0f : 0.0f);
#endif

        }
      }
    } else {
      setVolume();
    }

  }

  void setVolume() {
    for (auto device: devices) {
      for (size_t i = 0; i < device.numChannels; i++) {
  #ifdef USE_CURL
        std::string address = "http://" + device.host + "/datastore/ext/";
        address += "obank/" + std::to_string(device.bankIndex) + "/";
        address += "ch/" + std::to_string(i) + "/trim";
        std::string data = "json={\"value\":" + std::to_string(int(volume)) + "}" ;
        httpPost(address, data);
  #else
        osc::Send sender(device.oscPort.first, device.host.c_str());
        std::string address = "/mix/chan/" + std::to_string(i) + "/matrix/mute";
        sender.send(address, isMute ? 1.0f : 0.0f);
  #endif
      }
    }
  }

  void setLevel(float level) {

  }

#ifdef USE_CURL
  // HTTP requests
  void httpPost(std::string address, std::string data) {
      CURL *curl_handle;
      CURLcode res;

      curl_global_init(CURL_GLOBAL_ALL);

      curl_handle = curl_easy_init();
      curl_easy_setopt(curl_handle, CURLOPT_URL, address.c_str());
//      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
//      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.data());
      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, -1L);
      curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

      res = curl_easy_perform(curl_handle);
  }

//  static size_t
//  WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
//  {
//    size_t realsize = size * nmemb;
//    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

//    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
//    if(ptr == nullptr) {
//      /* out of memory! */
//      printf("not enough memory (realloc returned NULL)\n");
//      return 0;
//    }

//    mem->memory = ptr;
//    memcpy(&(mem->memory[mem->size]), contents, realsize);
//    mem->size += realsize;
//    mem->memory[mem->size] = 0;

//    std::cout << mem->memory <<std::endl;
//    return realsize;
//  }
#endif

};

int main() {
  MyApp app;
  app.dimensions(600, 400);
  app.start();
}
