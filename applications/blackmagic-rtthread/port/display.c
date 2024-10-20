#include "general.h"
#include "gdb_main.h"
#include "target_internal.h"
#include "ui.h"

/*
 Shows the target name on the display.
 */

void display_target_name()
{
	if (!cur_target)
		set_target_text("No target");
	else if (cur_target && cur_target->driver)
		set_target_text(cur_target->driver); // display target name
	else
		set_target_text("");
}
