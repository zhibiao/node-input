#include <napi.h>
#include <windows.h>

struct AddonContext
{
  DWORD threadId;
  bool suppress = false;
  Napi::ThreadSafeFunction cb;
};

AddonContext *context = new AddonContext();

LRESULT HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode < HC_ACTION)
  {
    return CallNextHookEx(NULL, nCode, wParam, lParam);
  }

  context->cb.BlockingCall(
      (void *)NULL,
      [wParam, lParam](Napi::Env env, Napi::Function jsCallback, void *data) {
        Napi::Object obj = Napi::Object::New(env);

        KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *)lParam;
        obj.Set("type", "keyboard-event");
        obj.Set("vkCode", kb->vkCode);
        obj.Set("scanCode", kb->scanCode);
        obj.Set("time", kb->time);

        if (wParam == WM_KEYDOWN)
        {
          obj.Set("status", 1);
        }
        else if (wParam == WM_KEYUP)
        {
          obj.Set("status", 0);
        }
        
        jsCallback.Call({obj});
      });

  if (context->suppress)
  {
    return TRUE;
  }
  else
  {
    return CallNextHookEx(NULL, nCode, wParam, lParam);
  }
}

DWORD ThreadProc(LPVOID lpParam)
{
  HHOOK hhook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, NULL, 0);
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  UnhookWindowsHookEx(hhook);
  delete context;
  return TRUE;
}

Napi::Value hook(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  Napi::Object options;
  Napi::Function callback;

  if (info.Length() == 1)
  {
    if (info[0].IsFunction())
    {
      callback = info[0].As<Napi::Function>();
    }
  }

  if (info.Length() == 2)
  {
    if (info[0].IsObject())
    {
      options = info[0].As<Napi::Object>();
    }

    if (info[1].IsFunction())
    {
      callback = info[1].As<Napi::Function>();
    }
  }

  if (!options.IsUndefined())
  {
    if (options.Has("suppress"))
    {
      context->suppress = options.Get("suppress").ToBoolean();
    }
  }

  if (!callback.IsUndefined())
  {
    context->cb = Napi::ThreadSafeFunction::New(
        env,
        callback,
        "HOOK",
        0,
        1,
        (void *)context,
        [](Napi::Env env, void *finalizeData, void *data) {
          PostThreadMessage(context->threadId, WM_QUIT, 0, 0);
        },
        (void *)NULL);

    HANDLE hThread = CreateThread(NULL, 0, ThreadProc, 0, 0, &context->threadId);
    CloseHandle(hThread);
  }

  return env.Null();
}

Napi::Value unhook(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  context->cb.Release();
  return env.Null();
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "hook"), Napi::Function::New(env, hook));
  exports.Set(Napi::String::New(env, "unhook"), Napi::Function::New(env, unhook));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)