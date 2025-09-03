#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

template <typename... Args>
class Debouncer {
   public:
	template <typename F>
	Debouncer(int delayMs, F&& callback) : delay(delayMs), func(std::forward<F>(callback)), active(false), stop(false) {
	}

	void call(Args... args) {
		std::lock_guard<std::mutex> lock(mtx);
		active = true;

		// Cancelar hilo anterior
		stop = true;
		if (worker.joinable())
			worker.join();
		stop = false;

		// Capturamos los argumentos por copia
		worker = std::thread([this, args...]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
			if (!stop) {
				std::lock_guard<std::mutex> lock(mtx);
				if (active) {
					active = false;
					func(args...);
				}
			}
		});
	}

	~Debouncer() {
		stop = true;
		if (worker.joinable())
			worker.join();
	}

   private:
	int delay;
	std::function<void(Args...)> func;
	std::thread worker;
	std::mutex mtx;
	std::atomic<bool> active;
	std::atomic<bool> stop;
};
