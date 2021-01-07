// cppcoro.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <coroutine>
#include <thread>
#include <future>

#include "cppcoro/single_consumer_event.hpp"
#include "cppcoro/sync_wait.hpp"
#include "cppcoro/task.hpp"

using cppcoro::task;
using namespace std::chrono;

/*
template<class Rep, class Period>
auto operator co_await(std::chrono::duration<Rep, Period> d)
{
	struct awaiter
	{
		std::chrono::system_clock::duration duration;
		// ...
		awaiter(std::chrono::system_clock::duration d)
			: duration(d)
		{
		}
		bool await_ready() const { return duration.count() <= 0; }
		void await_resume() {}
		void await_suspend(std::coroutine_handle<> h)
		{ // ...
		}
	};
	return awaiter{ d };
}
*/
// da https://stackoverflow.com/questions/49640336/implementing-example-from-coroutines-ts-2017
template<class Rep, class Period>
auto operator co_await(std::chrono::duration<Rep, Period> dur)
{
	struct awaiter
	{
		using clock = std::chrono::high_resolution_clock;
		clock::time_point resume_time;

		awaiter(clock::duration dur)
			: resume_time(clock::now() + dur)
		{
		}

		bool await_ready() { return resume_time <= clock::now(); }

		void await_suspend(std::coroutine_handle<> h)
		{
			std::thread([=]() {
				std::this_thread::sleep_until(resume_time);
				h.resume();  // destructs the obj, which has been std::move()'d
			}).detach();     // Detach scares me.
		}
		void await_resume() {}
	};

	return awaiter{ dur };
}

task<int> h()
{
	std::cout << "h - just about go to sleep...\n";
	co_await 10ms;
	std::cout << "h - resumed\n";
	co_return 1;
}

task<void> g()
{
	std::cout << "g - just about go to sleep...\n";
	co_await 10ms;
	std::cout << "g - resumed\n";
	co_await h();
}

task<void> sample()
{
	std::cout << "sample start\n";

	int x = co_await h();

	std::cout << "sample end\n";
}

task<int> deep_thought()
{
	co_await 7'500'000'000h;
	co_return 42;
}

int main()
{
	std::cout << "Hello coroutine!\n";

	//auto t = sample();

	cppcoro::sync_wait(sample());

	//auto con = std::async([] { cppcoro::sync_wait(sample()); });  // (1)
	//con.get();		

	//std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	std::cout << "Hello coroutine ended!\n";
}




