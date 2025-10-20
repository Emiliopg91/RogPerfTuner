#pragma once
#include <memory>
#include <mutex>

template <typename T>
class Singleton {
  public:
	template <typename... Args>
	static T& init(Args&&... args) {
		std::call_once(flag, [&]() {
			instance.reset(new T(std::forward<Args>(args)...));
		});
		return *instance;
	}

	static T& getInstance() {
		if (!instance) {
			init();
		}
		return *instance;
	}

  protected:
	Singleton()	 = default;
	~Singleton() = default;

	Singleton(const Singleton&)			   = delete;
	Singleton& operator=(const Singleton&) = delete;

  private:
	inline static std::unique_ptr<T> instance;
	inline static std::once_flag flag;
};
