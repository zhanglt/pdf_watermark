@echo off
SetLocal EnableDelayedExpansion
(set PATH=H:\Qt\qt5.15.10\qtbase\bin;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=H:\Qt\Qt5.15.10-release\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=H:\Qt\Qt5.15.10-release\plugins
)
H:\Qt\qt5.15.10\qtbase\bin\uic.exe %*
EndLocal
