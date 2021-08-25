// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

class FSharedHandle
{
public:
	FSharedHandle(HANDLE hInValue)
	{
		ptrValue = MakeShared<FHandleHolder, ESPMode::ThreadSafe>(hInValue);
	}

	FSharedHandle(std::nullptr_t = nullptr)
	{
		ptrValue = MakeShared<FHandleHolder, ESPMode::ThreadSafe>();
	}

	~FSharedHandle()
	{
		ptrValue = nullptr;
	}

	operator HANDLE () const
	{
		return ptrValue->hValue;
	}

	HANDLE* operator & ()
	{
		check(ptrValue->hValue == INVALID_HANDLE_VALUE);
		return &ptrValue->hValue;
	}

	bool IsValid() const
	{
		return ptrValue->IsValid();
	}

	operator bool () const
	{
		return IsValid();
	}

private:

	struct FHandleHolder : public FNoncopyable
	{
		HANDLE hValue;

		FHandleHolder(HANDLE hInValue)
		{
			hValue = hInValue;
		}

		FHandleHolder()
		{
			hValue = INVALID_HANDLE_VALUE;
		}

		bool IsValid() const
		{
			return hValue != INVALID_HANDLE_VALUE;
		}

		~FHandleHolder()
		{
			if (hValue != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hValue);
				hValue = INVALID_HANDLE_VALUE;
			}
		}

	};

	TSharedPtr<FHandleHolder, ESPMode::ThreadSafe> ptrValue;
};
