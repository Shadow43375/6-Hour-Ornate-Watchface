// void layer_set_update_proc(Layer * layer, LayerUpdateProc update_proc) when layer is dirtied function update_proc us called.
//     layer_mark_dirty(myCanvas); marks 'myCanvas' for update. 


#include <pebble.h>
#include "graphics.h"

//need to place hour and min hands on seperate layers 1) as possible fix for glitch bug and 2) to prevent transparency

static Window* s_main_window;
static Layer* myCanvas;
// static TextLayer *time_layer;
static Layer* motherLayer;
static BitmapLayer *clock_face_image_layer;
static GBitmap *Clock_Face;
static GPath *min_hand = NULL;
static GPath *hour_hand = NULL;
    
// instantiate useful values;
int seconds_in_day = 1*24*60*60;
int seconds_in_hour = 60*60;
int seconds_in_min = 60;


static void updateGame(Layer *layer, GContext *ctx)
{
//   GRect bounds = layer_get_bounds(layer);
  
  // Fill and stroke the paths for hour and min hands:
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  gpath_draw_filled(ctx, min_hand);
  gpath_draw_outline(ctx, min_hand); 
  gpath_draw_filled(ctx, hour_hand);
  gpath_draw_outline(ctx, hour_hand);
  
  // dot in the middle
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(73, 85, 3, 3), 0, GCornerNone);
}


void create_layout(int minute_hand_rotation, int hour_hand_rotation) {
    Layer *window_layer = window_get_root_layer(s_main_window);
    GRect bounds = layer_get_bounds(window_layer);
  
    clock_face_image_layer = bitmap_layer_create(bounds);
    Clock_Face = gbitmap_create_with_resource(RESOURCE_ID_Clock_Face);
  	
    bitmap_layer_set_compositing_mode(clock_face_image_layer, GCompOpAssign);
    bitmap_layer_set_bitmap(clock_face_image_layer, Clock_Face);
    bitmap_layer_set_alignment(clock_face_image_layer, GAlignCenter);
    layer_add_child(window_layer, bitmap_layer_get_layer(clock_face_image_layer));
  
     motherLayer = window_get_root_layer(s_main_window);
     GRect mother_bounds = layer_get_bounds(motherLayer);
     myCanvas = layer_create(GRect(0, 0, mother_bounds.size.w, mother_bounds.size.h));
     layer_add_child(motherLayer, myCanvas);
  
  // create the hour and min hands from data from simple_analog.h
     min_hand = gpath_create(&MINUTE_HAND_POINTS);    
     hour_hand = gpath_create(&HOUR_HAND_POINTS); 
     layer_set_update_proc(myCanvas, updateGame);
  
    //move hour and min hands into place (center)
      gpath_move_to(min_hand, GPoint (73, 85));
      
      gpath_move_to(hour_hand, GPoint (72, 85));

  
    //move hour and min hands into place
      gpath_move_to(min_hand, GPoint (73, 85));
      gpath_rotate_to(min_hand, minute_hand_rotation);

  
      gpath_move_to(hour_hand, GPoint (72, 85));
      gpath_rotate_to(hour_hand, hour_hand_rotation);


}



void update_hands(int minute_hand_rotation, int hour_hand_rotation) {
    //move hour and min hands into place
      gpath_move_to(min_hand, GPoint (73, 85));
      gpath_rotate_to(min_hand, minute_hand_rotation);

      gpath_move_to(hour_hand, GPoint (72, 85));
      gpath_rotate_to(hour_hand, hour_hand_rotation);

      layer_mark_dirty(myCanvas);
  
}





static void update_time() {
   int minute_hand_rotation = 0;
   int hour_hand_rotation = 0;
   
  
   // Get a tm structure
  time_t currentTime;
  time(&currentTime);
    
  //time struct for local time (important for getting local day/date).
  struct tm* timeDataLocal = localtime(&currentTime);
  

// calculate rotations for hour and min hands

  minute_hand_rotation = (TRIG_MAX_ANGLE * timeDataLocal->tm_min / 60);
//   hour_hand_rotation = (TRIG_MAX_ANGLE * (timeDataLocal->tm_hour%6) / 6);  
  hour_hand_rotation = (TRIG_MAX_ANGLE * (((timeDataLocal->tm_hour%6) * seconds_in_hour) + (timeDataLocal->tm_min * seconds_in_min))/(seconds_in_day/4)); 
  
//   create_layout(minute_hand_rotation, hour_hand_rotation);
  update_hands(minute_hand_rotation, hour_hand_rotation);
  
}



static void tick_handler(struct tm *timeData, TimeUnits units_changed) {
  update_time();
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap Available: %d", heap_bytes_free());
}


static void main_window_load(Window *window) {
  // Create Layers
  create_layout(0, 0);
}


static void main_window_unload(Window *window) {
  // Destroy Layers
}


static void init() {
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
//   window_set_background_color(s_main_window, GColorBlack);
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  // Destroy Window

  gpath_destroy(min_hand);
  gpath_destroy(hour_hand);


  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}