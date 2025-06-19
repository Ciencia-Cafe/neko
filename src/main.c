// Copyright 2025 Elloramir.
// Use of this source code is governed by a MIT
// license that can be found in the LICENSE file.

#include "system.h"
#include "renderer.h"

int entry_point ( void ) {
	system_create_window(800, 600, "Neko");
	renderer_init();

	while (!system_window_should_close()) {
		if (system_window_is_visible()) {
			renderer_frame();
			renderer_set_color((Color){ 1, 0, 0, 1 });
			renderer_push_quad(0.f, 0.f, 250.f, 250.f, 0.f, 1.f, 0.f, 1.f);
			renderer_flush();
			system_swap_buffers();
		}
		else {
			system_sleep(1);
		}
	}
	return 0;
}