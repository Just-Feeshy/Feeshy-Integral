#pragma once

static const char* menu_items[] = {
    "Ultra Low",
    "Low",
    "Medium",
    "High",
    "Ultra High"
};

static int selected_menu_item = 2;

// For World Module
void world_reset_camera();
void world_update_fov(float fov);
