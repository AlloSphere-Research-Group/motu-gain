#include <iostream>

#include "al/app/al_App.hpp"
#include "al/app/al_GUIDomain.hpp"
#include "al/ui/al_ParameterGUI.hpp"

#define USE_CURL

struct AVBDevice {
  std::string host;
  uint16_t oscPort;
  int bankIndex;
  size_t numChannels = 64;
};

using namespace al;

struct MyApp : App {

  ParameterBool mute{"Mute"};
  Trigger volumeUp{"Up"};
  float volume = -12.0;
  Trigger volumeDown{"Down"};
  Trigger minus18{"-18dB"};
  Trigger minus12{"-12dB"};
  Trigger minus6{"-6dB"};

  std::vector<AVBDevice> devices = {
      {"16A.local", 9998, 0, 16},
      {"24Ao.local", 9998, 0, 24},
      {"24Ao-2.local", 9998, 0, 24},
  };
  std::vector<std::string> simulatorMachines = {"localhost", "audio.1g",
                                                "ar01.1g"};

  void onInit() override {
    auto guiDomain = GUIDomain::enableGUI(defaultWindowDomain());
    auto panel = guiDomain->newPanel();
    panel->setFlags(ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    panel->guiConfigurationCode = [this, panel]() {
      panel->setDimensions(width(), height());
    };
    panel->guiCode = [this]() {
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

      ParameterGUI::draw(&minus6);
      ImGui::SameLine();
      ParameterGUI::draw(&minus12);
      ImGui::SameLine();
      ParameterGUI::draw(&minus18);
    };
  }

  void onCreate() override {

    mute.registerChangeCallback([&](float value) { setMute(value == 1.0f); });

    volumeUp.registerChangeCallback([&](float /*value*/) {
      volume++;
      sendVolume();
    });
    volumeDown.registerChangeCallback([&](float /*value*/) {
      volume--;
      sendVolume();
    });

    minus18.registerChangeCallback([&](float /*value*/) { setLevel(-18); });
    minus12.registerChangeCallback([&](float /*value*/) { setLevel(-12); });
    minus6.registerChangeCallback([&](float /*value*/) { setLevel(-6); });
  }

  void onAnimate(double dt) override {}

  void onDraw(Graphics &g) override { g.clear(0, 0, 0); }

  void onExit() override {}

#ifdef USE_CURL

  struct MemoryStruct {
    char *memory;
    size_t size;
  };
#endif

  void setMute(bool isMute) {
    if (isMute) {
#ifdef USE_CURL
      for (const auto &device : devices) {
        for (size_t i = 0; i < device.numChannels; i++) {
          std::string address = device.host + "/datastore";
          std::string jsonData = "{ \"ext/obank/" +
                                 std::to_string(device.bankIndex) + "/ch/" +
                                 std::to_string(i) + "/trim\" : -60}";
          std::string command =
              "curl --data 'json=" + jsonData + "' " + address;
          system(command.c_str());
        }
      }
#else
      for (const auto &device : devices) {
        osc::Send sender;
        if (sender.open(device.oscPort, device.host.c_str()))
          for (size_t i = 0; i < device.numChannels; i++) {

            std::string address = "/ext/";
            address += "obank/" + std::to_string(device.bankIndex) + "/";
            address += "ch/" + std::to_string(i) + "/trim";
            sender.send(address, 0.0f);
          }
      }
#endif
    } else {
      sendVolume();
    }
  }

  void sendVolume() {
#ifdef USE_CURL
    for (const auto &device : devices) {
      for (size_t i = 0; i < device.numChannels; i++) {

        std::string address = device.host + "/datastore";
        std::string jsonData = "{ \"ext/obank/" +
                               std::to_string(device.bankIndex) + "/ch/" +
                               std::to_string(i) +
                               "/trim\" : " + std::to_string(int(volume)) + "}";
        std::string command = "curl --data 'json=" + jsonData + "' " + address;
        system(command.c_str());
      }
    }
#else

    for (const auto &device : devices) {
      osc::Send sender;
      if (sender.open(device.oscPort, device.host.c_str())) {

        for (size_t i = 0; i < device.numChannels; i++) {
          std::string address = "/ext/";
          address += "obank/" + std::to_string(device.bankIndex) + "/";
          address += "ch/" + std::to_string(i) + "/trim";
          sender.send(address, 20.0f * std::powf(10.0f, volume / 20.0f));
        }
      }
    }

#endif
  }

  void setLevel(float level) {
    volume = level;
    sendVolume();
  }

#ifdef USE_CURL
  // HTTP requests
//    void httpPost(std::string address, std::string data) {
//      CURL *curl_handle;
//      CURLcode res;

//      curl_global_init(CURL_GLOBAL_ALL);

//      curl_handle = curl_easy_init();
//      curl_easy_setopt(curl_handle, CURLOPT_URL, address.c_str());
//      //      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,
//      //      WriteMemoryCallback); curl_easy_setopt(curl_handle,
//      //      CURLOPT_WRITEDATA, (void *)&chunk);
//      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.data());
//      curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, -1L);
//      curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

//      res = curl_easy_perform(curl_handle);
//    }

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
  app.configureAudio(48000, 512, 0,
                     0); // 0 channels makes the domain not run
  app.dimensions(600, 400);
  app.start();
}
