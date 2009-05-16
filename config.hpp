//==============================================================================

#ifndef ArseConfig
#define ArseConfig

#pragma once

#include <string>

class Config
{
  public:
    Config();
    virtual ~Config();

    void init();
    void fini();

    int screenWidth;
    int screenHeight;
    bool fullScreen;
    std::string userName;
    int menu;
    bool vibrate;
    bool leaderboard;
};

#endif

//==============================================================================
