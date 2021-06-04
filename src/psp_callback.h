
// �z�[���{�^���I�����ɃR�[���o�b�N
int exit_callback(void)
{
	sceKernelExitGame();
	return 0;
}

// �X���[�v����s����ɃR�[���o�b�N
void power_callback(int unknown, int pwrflags)
{
	// �R�[���o�b�N�֐��̍ēo�^
	// �i��x�Ă΂ꂽ��ēo�^���Ƃ��Ȃ��Ǝ��ɃR�[���o�b�N����Ȃ��j
	int cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);
}

// �|�[�����O�p�X���b�h
int CallbackThread(int args, void *argp)
{
	int cbid;
	
	// �R�[���o�b�N�֐��̓o�^
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);
	
	// �|�[�����O
	sceKernelPollCallbacks();
}

int SetupCallbacks(void)
{
	int thid = 0;
	
	// �|�[�����O�p�X���b�h�̐���
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	
	return thid;
}
