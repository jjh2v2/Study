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