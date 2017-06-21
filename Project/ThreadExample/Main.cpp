#include "windows.h"
#include<process.h>

/*
voidOnRender_MainThread()
{
	// 각 자식 렌더링 스레드에 알리고 렌더링을 시작합니다.
	forworkerIdinworkerIdList
	{
		SetEvent(BeginRendering_Events[workerId]);
	}

	// Pre 명령 목록은 렌더링을 준비하는 데 사용됩니다.
	// Pre 명령 목록 재설정
	pPreCommandList->Reset(...);

	// 백 버퍼의 표시 상태와 Rendering Target 사이의 장벽을 설정합니다.
	// 백 버퍼가 렌더 타겟으로 사용될 것임을 나타냅니다.
	pPreCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 백 버퍼의 색을 지운다.
	pPreCommandList->ClearRenderTargetView(...);

	// 백 버퍼의 깊이 / 템플릿 지우기
	pPreCommandList->ClearDepthStencilView(...);

	// Pre 명령리스트의 다른 연산들
	// ...

	// Pre 명령 목록 닫기
	pPreCommandList->Close();

	// Post 명령 목록은 렌더링 후에 끝내기 위해 사용됩니다.
	// 백 버퍼의 표시 상태와 Rendering Target 사이의 장벽을 설정합니다.
	// 백 버퍼가 렌더 타겟에서 프리젠테이션(present 표시 표시 상태)으로 나타냅니다.
	pPostCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Post 명령 목록의 다른 연산들
	// ...

	// Post 명령 목록 닫기
	pPostCommandList->Close();

	// Pre 명령 목록 제출
	pCommandQueue->ExecuteCommandLists(..., pPreCommandList);

	// 모든 작업자 스레드가 완료 될 때까지 대기 Task1
	// Task1 쓰레드가 끝날때까지 모두 대기 한다
	WaitForMultipleObjects(Task1_Events);

	// 모든 작업자 스레드에 대해 Task1의 명령 목록을 제출합니다.
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTask1);

	// 모든 작업자 스레드가 완료 될 때까지 대기 Task2
	// Task2 쓰레드가 끝날때까지 모두 대기 한다
	WaitForMultipleObjects(Task2_Events);

	// 완료된 렌더링 명령을 제출합니다 (모든 작업자 스레드에 대해 Task2의 명령 목록)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTask2);

	// ...
	// 모든 작업자 스레드가 TaskN을 완료 할 때까지 기다립니다.
	WaitForMultipleObjects(TaskN_Events);

	// 완료된 렌더링 명령을 제출합니다 (모든 작업자 스레드에 대해 TaskN의 명령 목록)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTaskN);

	// 마지막 명령 목록을 제출합니다 (pPostCommandList).
	pCommandQueue->ExecuteCommandLists(..., pPostCommandList);

	// SwapChain 프리젠 테이션 사용
	pSwapChain->Present(...);
}


voidOnRender_WorkerThread(workerId)
{
	// 각 루프는 자식 스레드의 한 프레임 렌더링을 나타냅니다.
	while (running)
	{
		// 메인 프레임에서 이벤트 알림을 기다리면 한 프레임 렌더링이 시작됩니다.
		WaitForSingleObject(BeginRendering_Events[workerId]);

		// Rendering subtask1
		{
			pCommandList1->SetGraphicsRootSignature(...);
			pCommandList1->IASetVertexBuffers(...);
			pCommandList1->IASetIndexBuffer(...);
			//...
			pCommandList1->DrawIndexedInstanced(...);
			pCommandList1->Close();

			// 현재 작업자 스레드의 렌더링 하위 작업 1이 완료되었음을 주 스레드에 알립니다.
			SetEvent(Task1_Events[workerId]);
		}

		// Rendering subtask2
		{
			pCommandList2->SetGraphicsRootSignature(...);
			pCommandList2->IASetVertexBuffers(...);
			pCommandList2->IASetIndexBuffer(...);
			//...
			pCommandList2->DrawIndexedInstanced(...);
			pCommandList2->Close();

			// 현재 작업자 스레드의 렌더링 하위 작업 2가 완료되었음을 주 스레드에 알립니다.
			SetEvent(Task2_Events[workerId]);
		}

		// 더 많은 렌더링 하위 작업
		//...

		// Rendering subtaskN
		{
			pCommandListN->SetGraphicsRootSignature(...);
			pCommandListN->IASetVertexBuffers(...);
			pCommandListN->IASetIndexBuffer(...);
			//...
			pCommandListN->DrawIndexedInstanced(...);
			pCommandListN->Close();

			// 현재 작업자 스레드의 렌더링 하위 작업 N이 완료되었음을 주 스레드에 알립니다.
			SetEvent(TaskN_Events[workerId]);
		}
	}
}
*/

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