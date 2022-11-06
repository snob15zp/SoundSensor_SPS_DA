#include "ss_global.h"

uint8_t dummy_function_status;

e_FunctionReturnState  FSM_Transition(t_s_FSM * FSM, key_type key)
{
//	static uint8_t state=0;
//	static uint8_t state1=0;
	e_FunctionReturnState rstate,rstatel;
	
	
	
	rstate=e_FRS_Not_Done;
while(1)
	{	
		if (((key>>FSM->state)&1)!=0)
			{ rstatel=FSM->fs(FSM);
              if (e_FRS_Done==rstatel)   //switch ?
                                      {FSM->state++;};
              if (e_FRS_DoneError==rstatel)
									{ FSM->mFSM_Error|=1<<FSM->state;  // Tries to do everything regardless of mistakes
										FSM->state++;
									};
              if (e_FRS_Not_Done==rstatel)
								{ break;
								};
	        }
	        else
	        {FSM->state++;};
		     if (FSM->state>=FSM->NumOfel)  
					{ 
						if (FSM->mFSM_Error==0) 
						{rstate=e_FRS_Done;}
						else
						{rstate=e_FRS_DoneError;};
					FSM->state=0;
					FSM->mFSM_Error=0;
					break;	//exit
					};   
	};
	return rstate;
}

//e_FSM_State mainFMSstate=e_FSM_Init;
//static e_FSM_State sign=e_FSM_RestOff;


e_FunctionReturnState  FSM_main(t_s_FSM *FSM)
{
 static uint8_t state=0;

 e_FunctionReturnState rstate;
 e_FunctionReturnState rstatel;
 rstate=e_FRS_Not_Done;
 switch (state)
 {
  case 0: rstatel=FSM_Transition(FSM, FSM->keys[(FSM->sign)*(FSM->keyrows)+(FSM->mainFMSstate)]); //TransitionKeys
          if (e_FRS_Done==rstatel)
             { FSM->mainFMSstate=FSM->sign;
               state++;
//               rstate=e_FRS_Done;
             };
          if (e_FRS_DoneError==rstatel)   //if error on transition, go to previos state ????
             { state++;
//               rstate=e_FRS_Done;
             };
          break;
  case 1: rstatel=FSM_Transition(FSM, FSM->keys[(FSM->mainFMSstate)*(FSM->keyrows)+(FSM->mainFMSstate)]);    //Steady state
          if ((e_FRS_Done==rstatel)||(e_FRS_DoneError==rstatel))                     //sign is renewed here
          { if (FSM->mainFMSstate!=FSM->sign)                                                  
            {state++;
            };
            rstate=rstatel;
          };
          break;
  default: state=0;
 };

 return rstate;

}

