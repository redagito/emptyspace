#pragma once

struct NonCopyable
{
public:
	NonCopyable(const NonCopyable&) = delete;
	NonCopyable(NonCopyable&&) = delete;
protected:
	NonCopyable() = default;
	virtual ~NonCopyable() = default;

	NonCopyable& operator=(const NonCopyable&) = default;
	NonCopyable& operator=(NonCopyable&&) = default;
};