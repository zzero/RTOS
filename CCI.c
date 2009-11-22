void CCI()
{
     MsgEnv* CCI_env;    
     CCI_env->msg_txt = "CCI:"
	 send_console_chars(CCI_env)
	 CCI_env =  receive_message()
	
     if (CCI_env.msg_type == DISPLAY_ACKNOWLEDGE)
	 {  
       get_console_chars(CCI_env)
	   CCI_env = receive_message()
     }
	 
 }
