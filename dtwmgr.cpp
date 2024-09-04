#define UNICODE
#define _UNICODE
#include <Windows.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

std::atomic<bool> keepSecreting(false);
std::atomic<bool> secretEnabled(false);

void DoSecret() {
   INPUT input = {0};
   input.type = INPUT_MOUSE;
   input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
   SendInput(1, &input, sizeof(INPUT));

   input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
   SendInput(1, &input, sizeof(INPUT));
}

void Secret() {
   using namespace std::chrono;
   auto next_secret = steady_clock::now();

   while (true) {
      if (secretEnabled.load() && keepSecreting.load()) {
         auto now = steady_clock::now();
         if (now >= next_secret) {
            DoSecret();
            next_secret = now + duration_cast<steady_clock::duration>(
                                    duration<double, std::milli>(46.76));
         } else {
            std::this_thread::sleep_for(milliseconds(10));
         }
      } else {
         std::this_thread::sleep_for(milliseconds(10));
      }
   }
}

void ToggleSecret() {
   while (true) {
      if (GetAsyncKeyState(VK_CONTROL) & 0x8000 &&
          GetAsyncKeyState('0') & 0x8000) {
         secretEnabled.store(!secretEnabled.load());
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
   }
}

int main() {
   HANDLE mutex = CreateMutex(NULL, TRUE, L"dtwMutex");
   if (GetLastError() == ERROR_ALREADY_EXISTS) {
      return 1;
   }

   std::thread secretThread(Secret);
   std::thread toggleThread(ToggleSecret);

   while (true) {
      if (GetKeyState(VK_XBUTTON2) & 0x8000) {
         keepSecreting.store(true);
      } else {
         keepSecreting.store(false);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
   }

   secretThread.join();
   toggleThread.join();

   ReleaseMutex(mutex);
   CloseHandle(mutex);

   return 0;
}
