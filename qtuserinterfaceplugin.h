#ifndef QTUSERINTERFACEPLUGIN_H
#define QTUSERINTERFACEPLUGIN_H

#include "Unity/IUnityInterface.h"
#include "Unity/IUnityGraphics.h"

extern "C" {

// If exported by a plugin, this function will be called when the plugin is loaded.
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
// If exported by a plugin, this function will be called when the plugin is about to be unloaded.
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();

// Plugin callbacks
UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc();
UnityRenderingEventAndData UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventAndDataFunc();

// Qt Related
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UpdateQtEventLoop();

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTimeFromUnity(float time);
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTextureFromUnity(void *textureHandle, int width, int height);
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RegisterTouchStartEvent(float x, float y, int touchpoint);
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RegisterTouchEndEvent(float x, float y, int touchpoint);
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RegisterTouchMoveEvent(float x, float y, int touchpoint);



}


#endif // QTUSERINTERFACEPLUGIN_H
