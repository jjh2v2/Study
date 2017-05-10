#include "windows.h"
#include<process.h>


struct ThreadParameter
{
	int threadIndex;
};

static const UINT threadNum = 4;
HANDLE m_threadHandles[ threadNum ];
HANDLE m_workerBeginRenderFrame[ threadNum ];
ThreadParameter m_threadParameters[ threadNum ];

unsigned int WINAPI WorkerThread( LPVOID lpParam )
{
	ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParam);
	int threadI = parameter->threadIndex;
	while ( threadI >= 0 && threadI < threadNum )
	{
		WaitForSingleObject( m_workerBeginRenderFrame[ threadI ], INFINITE );
		int iiiiii = 0;
	}


	return 0;
}

void main()
{
	for(int i = 0; i < threadNum; i++)
	{
		m_threadParameters[ i ].threadIndex = i;

		/*
		_beginthreadex(
		1.        LPSECURITY_ATTRIBUTES lpThreadAttributes,
		2.        SIZE_T dwStackSize,
		3.        LPTHREAD_START_ROUTINE lpStartAddress,
		4.        LPVOID lpParameter,
		5.        DWORD dwCreationFlags,
		6.        LPDWORD lpThreadId        );

		1. LPSECURITY_ATTRIBUTES lpThreadAttributes,
		SECURITY_ATTRIBUTES 구조체는,
		생성하는 핸들에 대해 보안속성을

		2. SIZE_T dwStackSize //dw는 DWORD겠죠 ㅎㅎ
		쓰래드는 고유의 스택을 가지고 있습니다. 스택 크기를 지정합니다.
		0 (또는 NULL) :: Default 값으로 1mb가 적용됩니다.

		3. LPTHREAD_START_ROUTINE lpStartAddress

		쓰레드가 작동할 함수의 이름을 넣으시면 됩니다.
		typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)
							(LPVOID lpThreadParameter);
		typedef PTHREAD_START_ROUTINE    LPTHREAD_START_ROUTINE;

		함수 예)
		DWORD WINAPI ThreadEx(LPVOID lpParameter){
				return 0;
		}

		4. LPVOID lpParameter
		함수의 인자로 넘어가는것과 같습니다.
		더블 포인터도 가능합니다.

		5. DWORD dwCreationFlags, //Flag 입니다.
		CREATE_SUSPEND
			:: suspend count 1로 설정 ( 스레드 priority control 관련글 참고 링크 :: LINK_ )
				:: suspend count가 0이 되기 전까지는, 스레드는 동작하지 않습니다.
				이 인자를 넣을 시에, 원하는 시기에 스레드를 시작할 수 있습니다.
				DWORD ResumeThread(HANDLE hThread)   ::  Suspend Count 1 감소
				DWORD SuspendThread(HANDLE hThread)   ::  Suspend Count 1 증가

		STACK_SIZE_PARAM_IS_A_RESERVATION

				:: Reserve stack size를 변경하려면 위 플레그를 추가 한 후
				스레드 생성 함수들의 매개변수 dwStackSize파라미터를 사용한다.

		아래는 CreationFlag이지만, 프로세스에서만 쓰인다.
		CREATE_NEW_CONSOLE
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS

		6. LPDWORD lpThreadId
		생성시에 이 변수로 쓰레드ID가 전달됩니다.
		필요 없다면 NULL.

		return :: HANDLE.
		CreateThread 함수의 리턴값은, 스레드를 가리키는 핸들

		스레드는 독립된 스택을 할당받기 때문에 메모리를 차지하게 됩니다.
		메모리가 허용하는 만큼 스레드 생성이 가능합니다.
		*/
		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr,
			0,
			WorkerThread,
			reinterpret_cast<LPVOID>(&m_threadParameters[ i ]),
			0,
			nullptr ));

		m_workerBeginRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL );
	}

	while (true)
	{
		for ( int i = 0; i < threadNum; i++ )
		{
			SetEvent( m_workerBeginRenderFrame[ i ] );
		}
	}

	for ( int i = 0; i < threadNum; i++ )
	{
		CloseHandle( m_workerBeginRenderFrame[ i ] );
		CloseHandle( m_threadHandles[ i ] );
	}
}