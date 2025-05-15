//
// Created by radue on 3/1/2025.
//
#pragma once

template <typename U>
class EngineWrapper {
public:
	virtual ~EngineWrapper() = default;

	U operator*() const { return m_handle; }
	const U* operator->() const { return &m_handle; }

protected:
	U m_handle;
};
