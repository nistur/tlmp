#include "tlmp-tests.h"
#include "tlmp.h"

TEST(InitTerminate, Basic, 0.0f,
     // initialisation
     {
	 m_data.context = 0;
     },
     // cleanup
     {
	 tlmpTerminateContext(&m_data.context);
     },
     // test
     {
	 ASSERT(tlmpInitContext(&m_data.context) == TLMP_SUCCESS);
      ASSERT(m_data.context != 0);
	 ASSERT(tlmpTerminateContext(&m_data.context) == TLMP_SUCCESS);
      ASSERT(m_data.context == 0)
     },
     // data
     {
	 tlmpContext* context;
     }
    );
