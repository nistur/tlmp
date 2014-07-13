#include "template_internal.h"

tmplReturn tmplClearContext(tmplContext* context)
{
    tmplReturn(SUCCESS);
}

tmplReturn tmplInitContext(tmplContext** context)
{
    if(context == 0)
        tmplReturn(NO_CONTEXT);
    *context = tmplMalloc(tmplContext);
    if(tmplClearContext(*context) != TMPL_SUCCESS)
	   tmplTerminateContext(context);
    tmplReturn(SUCCESS);
}

tmplReturn tmplTerminateContext(tmplContext** context)
{
    if(*context == 0)
	   tmplReturn(NO_CONTEXT);

    tmplFree(*context);
    *context = 0;
    tmplReturn(SUCCESS);
}

const char* tmplError()
{
    return g_tmplErrors[g_tmplError];
}
