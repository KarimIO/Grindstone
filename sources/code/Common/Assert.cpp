#include <iostream>
#include <mutex>

#include <Windows.h>

#include "Assert.hpp"

std::mutex handlerMutex;
Grindstone::Debug::AssertHandler currentHandler = &Grindstone::Debug::DefaultAssertHandler;

Grindstone::Debug::AssertHandler Grindstone::Debug::SetAssertHandler(AssertHandler handler) {
	std::lock_guard lock(handlerMutex);
	AssertHandler previous = std::move(currentHandler);
	currentHandler = handler ? std::move(handler) : AssertHandler(&DefaultAssertHandler);
	return previous;
}

Grindstone::Debug::AssertAction Grindstone::Debug::ReportAssert(const AssertInfo& info) {
	AssertHandler handler;
	{
		std::lock_guard lock(handlerMutex);
		handler = currentHandler;
	}
	return handler(info);  // called outside the lock so a handler may throw
}

Grindstone::Debug::AssertAction Grindstone::Debug::DefaultAssertHandler(const AssertInfo& info) {
	const std::string full =
		std::format("{}({}): Assertion Failed\nFunction: {}\n\n{}",
					info.location.file_name(), info.location.line(),
					info.location.function_name(), info.message);

	std::cerr << full << std::endl;

#ifdef _WIN32
	// Skip the modal dialog on CI or when explicitly disabled.
	if (!std::getenv("GS_ASSERT_NO_DIALOG")) {
		const int wideSize = ::MultiByteToWideChar(
			CP_UTF8,
			0,
			full.data(),
			static_cast<int>(full.size()),
			nullptr,
			0
		);

		std::wstring wideMessage(wideSize, L'\0');

		::MultiByteToWideChar(
			CP_UTF8,
			0,
			full.data(),
			static_cast<int>(full.size()),
			wideMessage.data(),
			wideSize
		);

		::MessageBoxW(
			nullptr,
			wideMessage.c_str(),
			L"Assertion Failed",
			MB_ICONEXCLAMATION | MB_OK
		);
	}
#endif

	return AssertAction::Break;
}
