#include "windows.h"
#include<process.h>

/*
voidOnRender_MainThread()
{
	// �� �ڽ� ������ �����忡 �˸��� �������� �����մϴ�.
	forworkerIdinworkerIdList
	{
		SetEvent(BeginRendering_Events[workerId]);
	}

	// Pre ��� ����� �������� �غ��ϴ� �� ���˴ϴ�.
	// Pre ��� ��� �缳��
	pPreCommandList->Reset(...);

	// �� ������ ǥ�� ���¿� Rendering Target ������ �庮�� �����մϴ�.
	// �� ���۰� ���� Ÿ������ ���� ������ ��Ÿ���ϴ�.
	pPreCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// �� ������ ���� �����.
	pPreCommandList->ClearRenderTargetView(...);

	// �� ������ ���� / ���ø� �����
	pPreCommandList->ClearDepthStencilView(...);

	// Pre ��ɸ���Ʈ�� �ٸ� �����
	// ...

	// Pre ��� ��� �ݱ�
	pPreCommandList->Close();

	// Post ��� ����� ������ �Ŀ� ������ ���� ���˴ϴ�.
	// �� ������ ǥ�� ���¿� Rendering Target ������ �庮�� �����մϴ�.
	// �� ���۰� ���� Ÿ�ٿ��� ���������̼�(present ǥ�� ǥ�� ����)���� ��Ÿ���ϴ�.
	pPostCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Post ��� ����� �ٸ� �����
	// ...

	// Post ��� ��� �ݱ�
	pPostCommandList->Close();

	// Pre ��� ��� ����
	pCommandQueue->ExecuteCommandLists(..., pPreCommandList);

	// ��� �۾��� �����尡 �Ϸ� �� ������ ��� Task1
	// Task1 �����尡 ���������� ��� ��� �Ѵ�
	WaitForMultipleObjects(Task1_Events);

	// ��� �۾��� �����忡 ���� Task1�� ��� ����� �����մϴ�.
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTask1);

	// ��� �۾��� �����尡 �Ϸ� �� ������ ��� Task2
	// Task2 �����尡 ���������� ��� ��� �Ѵ�
	WaitForMultipleObjects(Task2_Events);

	// �Ϸ�� ������ ����� �����մϴ� (��� �۾��� �����忡 ���� Task2�� ��� ���)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTask2);

	// ...
	// ��� �۾��� �����尡 TaskN�� �Ϸ� �� ������ ��ٸ��ϴ�.
	WaitForMultipleObjects(TaskN_Events);

	// �Ϸ�� ������ ����� �����մϴ� (��� �۾��� �����忡 ���� TaskN�� ��� ���)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTaskN);

	// ������ ��� ����� �����մϴ� (pPostCommandList).
	pCommandQueue->ExecuteCommandLists(..., pPostCommandList);

	// SwapChain ������ ���̼� ���
	pSwapChain->Present(...);
}


voidOnRender_WorkerThread(workerId)
{
	// �� ������ �ڽ� �������� �� ������ �������� ��Ÿ���ϴ�.
	while (running)
	{
		// ���� �����ӿ��� �̺�Ʈ �˸��� ��ٸ��� �� ������ �������� ���۵˴ϴ�.
		WaitForSingleObject(BeginRendering_Events[workerId]);

		// Rendering subtask1
		{
			pCommandList1->SetGraphicsRootSignature(...);
			pCommandList1->IASetVertexBuffers(...);
			pCommandList1->IASetIndexBuffer(...);
			//...
			pCommandList1->DrawIndexedInstanced(...);
			pCommandList1->Close();

			// ���� �۾��� �������� ������ ���� �۾� 1�� �Ϸ�Ǿ����� �� �����忡 �˸��ϴ�.
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

			// ���� �۾��� �������� ������ ���� �۾� 2�� �Ϸ�Ǿ����� �� �����忡 �˸��ϴ�.
			SetEvent(Task2_Events[workerId]);
		}

		// �� ���� ������ ���� �۾�
		//...

		// Rendering subtaskN
		{
			pCommandListN->SetGraphicsRootSignature(...);
			pCommandListN->IASetVertexBuffers(...);
			pCommandListN->IASetIndexBuffer(...);
			//...
			pCommandListN->DrawIndexedInstanced(...);
			pCommandListN->Close();

			// ���� �۾��� �������� ������ ���� �۾� N�� �Ϸ�Ǿ����� �� �����忡 �˸��ϴ�.
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
		SECURITY_ATTRIBUTES ����ü��,
		�����ϴ� �ڵ鿡 ���� ���ȼӼ���

		2. SIZE_T dwStackSize //dw�� DWORD���� ����
		������� ������ ������ ������ �ֽ��ϴ�. ���� ũ�⸦ �����մϴ�.
		0 (�Ǵ� NULL) :: Default ������ 1mb�� ����˴ϴ�.

		3. LPTHREAD_START_ROUTINE lpStartAddress

		�����尡 �۵��� �Լ��� �̸��� �����ø� �˴ϴ�.
		typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)
							(LPVOID lpThreadParameter);
		typedef PTHREAD_START_ROUTINE    LPTHREAD_START_ROUTINE;

		�Լ� ��)
		DWORD WINAPI ThreadEx(LPVOID lpParameter){
				return 0;
		}

		4. LPVOID lpParameter
		�Լ��� ���ڷ� �Ѿ�°Ͱ� �����ϴ�.
		���� �����͵� �����մϴ�.

		5. DWORD dwCreationFlags, //Flag �Դϴ�.
		CREATE_SUSPEND
			:: suspend count 1�� ���� ( ������ priority control ���ñ� ���� ��ũ :: LINK_ )
				:: suspend count�� 0�� �Ǳ� ��������, ������� �������� �ʽ��ϴ�.
				�� ���ڸ� ���� �ÿ�, ���ϴ� �ñ⿡ �����带 ������ �� �ֽ��ϴ�.
				DWORD ResumeThread(HANDLE hThread)   ::  Suspend Count 1 ����
				DWORD SuspendThread(HANDLE hThread)   ::  Suspend Count 1 ����

		STACK_SIZE_PARAM_IS_A_RESERVATION

				:: Reserve stack size�� �����Ϸ��� �� �÷��׸� �߰� �� ��
				������ ���� �Լ����� �Ű����� dwStackSize�Ķ���͸� ����Ѵ�.

		�Ʒ��� CreationFlag������, ���μ��������� ���δ�.
		CREATE_NEW_CONSOLE
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS

		6. LPDWORD lpThreadId
		�����ÿ� �� ������ ������ID�� ���޵˴ϴ�.
		�ʿ� ���ٸ� NULL.

		return :: HANDLE.
		CreateThread �Լ��� ���ϰ���, �����带 ����Ű�� �ڵ�

		������� ������ ������ �Ҵ�ޱ� ������ �޸𸮸� �����ϰ� �˴ϴ�.
		�޸𸮰� ����ϴ� ��ŭ ������ ������ �����մϴ�.
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