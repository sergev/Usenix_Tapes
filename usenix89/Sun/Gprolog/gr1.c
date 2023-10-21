
/*
 * Gprolog 1.4/1.5
 *
 * Barry Brachman
 * Dept. of Computer Science
 * Univ. of British Columbia
 * Vancouver, B.C. V6T 1W5
 *
 * .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
 * brachman@cs.ubc.cdn
 * brachman%ubc.csnet@csnet-relay.arpa
 * brachman@ubc.csnet
 */

#include "gr.h"

/* Interface test routines */

extern int list();
extern int test1(), test2(), test3(), test4(), test5(), test6(), test7();

/* Core functions */

/* int allocate_raster(); */
int await_any_button();
int await_any_button_get_locator_2();
int await_any_button_get_valuator();
int await_keyboard();
int await_pick();
/* int await_stroke_2(); */
int begin_batch_of_updates();
int close_retained_segment();
int close_temporary_segment();
int create_retained_segment();
int create_temporary_segment();
int define_color_indices();
int delete_all_retained_segments();
int delete_retained_segment();
int deselect_view_surface();
int end_batch_of_updates();
/* int file_to_raster(); */
/* int free_raster(); */
int get_mouse_state();
/* int get_raster(); */
int initialize_core();
int initialize_device();
int initialize_view_surface();
int inquire_charjust();
int inquire_charpath_2();
int inquire_charpath_3();
int inquire_charprecision();
int inquire_charsize();
int inquire_charspace();
int inquire_charup_2();
int inquire_charup_3();
/* int inquire_color_indices(); */
int inquire_current_position_2();
int inquire_current_position_3();
int inquire_detectability();
int inquire_echo();
int inquire_echo_position();
int inquire_echo_surface();
int inquire_fill_index();
int inquire_font();
int inquire_highlighting();
int inquire_image_transformation_2();
int inquire_image_transformation_3();
int inquire_image_transformation_type();
int inquire_image_translate_2();
int inquire_image_translate_3();
/* int inquire_inverse_composite_matrix(); */
int inquire_keyboard();
int inquire_line_index();
int inquire_linestyle();
int inquire_linewidth();
int inquire_locator_2();
int inquire_marker_symbol();
int inquire_ndc_space_2();
int inquire_ndc_space_3();
int inquire_open_retained_segment();
int inquire_open_temporary_segment();
int inquire_pen();
int inquire_pick_id();
int inquire_polygon_edge_style();
int inquire_polygon_interior_style();
/* int inquire_primitive_attributes(); */
int inquire_rasterop();
int inquire_projection();
/* int inquire_rasterop(); */
/* int inquire_retained_segment_names(); */
/* int inquire_retained_segment_surfaces(); */
int inquire_segment_detectability();
int inquire_segment_highlighting();
int inquire_segment_image_transformation_2();
int inquire_segment_image_transformation_3();
int inquire_segment_image_transformation_type();
int inquire_segment_image_translate_2();
int inquire_segment_image_translate_3();
int inquire_segment_visibility();
int inquire_stroke();
int inquire_text_extent_2();
int inquire_text_extent_3();
int inquire_text_index();
int inquire_valuator();
int inquire_view_depth();
int inquire_view_plane_distance();
int inquire_view_plane_normal();
int inquire_view_reference_point();
int inquire_view_up_2();
int inquire_view_up_3();
int inquire_viewing_control_parameters();
/* int inquire_viewing_parameters(); */
int inquire_viewport_2();
int inquire_viewport_3();
int inquire_visibility();
int inquire_window();
/* int inquire_world_coordinate_matrix_2(); */
/* int inquire_world_coordinate_matrix_3(); */
int line_abs_2();
int line_abs_3();
int line_rel_2();
int line_rel_3();
int map_ndc_to_world_2();
int map_ndc_to_world_3();
int map_world_to_ndc_2();
int map_world_to_ndc_3();
int marker_abs_2();
int marker_abs_3();
int marker_rel_2();
int marker_rel_3();
int move_abs_2();
int move_abs_3();
int move_rel_2();
int move_rel_3();
int new_frame();
int polygon_abs_2();
int polygon_abs_3();
int polygon_rel_2();
int polygon_rel_3();
int polyline_abs_2();
int polyline_abs_3();
int polyline_rel_2();
int polyline_rel_3();
int polymarker_abs_2();
int polymarker_abs_3();
int polymarker_rel_2();
int polymarker_rel_3();
int print_error();
/* int put_raster(); */
/* int raster_to_file(); */
int rename_retained_segment();
int report_most_recent_error();
int restore_segment();
int save_segment();
int select_view_surface();
int set_back_plane_clipping();
int set_charjust();		/* NOT YET IMPLEMENTED IN SUNCORE */
int set_charpath_2();
int set_charpath_3();
int set_charprecision();
int set_charsize();
int set_charspace();
int set_charup_2();
int set_charup_3();
int set_coordinate_system_type();
int set_detectability();
int set_drag();
int set_echo();
int set_echo_group();
int set_echo_position();
int set_echo_surface();
int set_fill_index();
int set_font();
int set_front_plane_clipping();
int set_highlighting();
int set_image_transformation_2();
int set_image_transformation_3();
int set_image_transformation_type();
int set_image_translate_2();
int set_image_translate_3();
int set_keyboard();
int set_light_direction();
int set_line_index();
int set_linestyle();
int set_linewidth();
int set_locator_2();
int set_marker_symbol();
int set_ndc_space_2();
int set_ndc_space_3();
int set_output_clipping();
int set_pen();
int set_pick();
int set_pick_id();
int set_polygon_edge_style();		/* NOT YET IMPLEMENTED IN SUNCORE */
int set_polygon_interior_style();
/* int set_primitive_attributes(); */
int set_projection();
int set_rasterop(); 
int set_segment_detectability();
int set_segment_highlighting();
int set_segment_image_transformation_2();
int set_segment_image_transformation_3();
int set_segment_image_translate_2();
int set_segment_image_translate_3();
int set_segment_visibility();
int set_shading_parameters();
int set_stroke();
int set_text_index();
int set_valuator();
int set_vertex_indices();
int set_vertex_normals();
int set_view_depth();
int set_view_plane_distance();
int set_view_plane_normal();
int set_view_reference_point();
int set_view_up_2();
int set_view_up_3();
/* int set_viewing_parameters(); */
int set_viewport_2();
int set_viewport_3();
int set_visibility();
int set_window();
int set_window_clipping();
/* int set_world_coordinate_matrix_2(); */
/* int set_world_coordinate_matrix_3(); */
int set_zbuffer_cut();
/* int size_raster(); */
int terminate_core();
int terminate_device();
int terminate_view_surface();
int text();

/*
 * The following are non-SunCore functions
 */
int getenv_mapper();

/*
struct Core_info {
	char *Core_name;
	int (*Core_func)();
	char Core_arity;
	char Core_arg_type[MAXARGS];
};
*/

struct Core_info Core_info[] = {

		/* Must be in alphabetical order! */

	{ "await_any_button", await_any_button,
		2, { INT_ARG, INT_PTR } },
	{ "await_any_button_get_locator_2", await_any_button_get_locator_2,
		5, { INT_ARG, INT_ARG, INT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "await_any_button_get_valuator", await_any_button_get_valuator,
		4, { INT_ARG, INT_ARG, INT_PTR, FLOAT_PTR } },
	{ "await_keyboard", await_keyboard,
		4, { INT_ARG, INT_ARG, STRING_PTR, INT_PTR } },
	{ "await_pick", await_pick,
		4, { INT_ARG, INT_ARG, INT_PTR, INT_PTR } },
/*	{ "await_stroke_2", await_stroke_2,
		6, { INT_ARG, INT_ARG, INT_ARG, FLOAT_VEC_PTR,
			FLOAT_VEC_PTR, INT_PTR } },
*/
	{ "begin_batch_of_updates", begin_batch_of_updates,
		0 },
	{ "close_retained_segment", close_retained_segment,
		0 },
	{ "close_temporary_segment", close_temporary_segment,
		0 },
	{ "create_retained_segment", create_retained_segment,
		1, { INT_ARG } },
	{ "create_temporary_segment", create_temporary_segment,
		0 },
	{ "define_color_indices", define_color_indices,
		6, { ADDR_ARG, INT_ARG, INT_ARG, FLOAT_VEC_ARG,
			FLOAT_VEC_ARG, FLOAT_VEC_ARG } },
	{ "delete_all_retained_segments", delete_all_retained_segments,
		0 },
	{ "delete_retained_segment", delete_retained_segment,
		1, { INT_ARG } },
	{ "deselect_view_surface", deselect_view_surface,
		1, { ADDR_ARG } },
	{ "end_batch_of_updates", end_batch_of_updates,
		0 },
	{ "get_mouse_state", get_mouse_state,
		5, { INT_ARG, INT_ARG, FLOAT_PTR, FLOAT_PTR, INT_PTR } },
/**/	{ "getenv", getenv_mapper,
		2, { STRING_ARG, STRING_PTR } },
	{ "initialize_core", initialize_core,
		3, { INT_ARG, INT_ARG, INT_ARG } },
	{ "initialize_device", initialize_device,
		2, { INT_ARG, INT_ARG } },
	{ "initialize_view_surface", initialize_view_surface,
		2, { ADDR_ARG, INT_ARG } },
	{ "inquire_charjust", inquire_charjust,
		1, { INT_PTR } },
	{ "inquire_charpath_2", inquire_charpath_2,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_charpath_3", inquire_charpath_3,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_charprecision", inquire_charprecision,
		1, { INT_PTR } },
	{ "inquire_charsize", inquire_charsize,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_charspace", inquire_charspace,
		1, { FLOAT_PTR } },
	{ "inquire_charup_2", inquire_charup_2,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_charup_3", inquire_charup_3,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_current_position_2", inquire_current_position_2,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_current_position_3", inquire_current_position_3,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_detectability", inquire_detectability,
		1, { INT_PTR } },
	{ "inquire_echo", inquire_echo,
		3, { INT_ARG, INT_ARG, INT_PTR } },
	{ "inquire_echo_position", inquire_echo_position,
		4, { INT_ARG, INT_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_echo_surface", inquire_echo_surface,
		3, { INT_ARG, INT_ARG, ADDR_PTR } },
	{ "inquire_fill_index", inquire_fill_index,
		1, { INT_PTR } },
	{ "inquire_font", inquire_font,
		1, { INT_PTR } },
	{ "inquire_highlighting", inquire_highlighting,
		1, { INT_PTR } },
	{ "inquire_image_transformation_2", inquire_image_transformation_2,
		5, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_image_transformation_3", inquire_image_transformation_3,
		9, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR,
			FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_image_transformation_type",
			inquire_image_transformation_type,
		1, { INT_PTR } },
	{ "inquire_image_translate_2", inquire_image_translate_2,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_image_translate_3", inquire_image_translate_3,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_keyboard", inquire_keyboard,
		4, { INT_ARG, INT_PTR, STRING_PTR, INT_PTR } },
	{ "inquire_line_index", inquire_line_index,
		1, { INT_PTR } },
	{ "inquire_linestyle", inquire_linestyle,
		1, { INT_PTR } },
	{ "inquire_linewidth", inquire_linewidth,
		1, { FLOAT_PTR } },
	{ "inquire_locator_2", inquire_locator_2,
		3, { INT_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_marker_symbol", inquire_marker_symbol,
		1, { INT_PTR } },
	{ "inquire_ndc_space_2", inquire_ndc_space_2,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_ndc_space_3", inquire_ndc_space_3,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_open_retained_segment", inquire_open_retained_segment,
		1, { INT_PTR } },
	{ "inquire_open_temporary_segment", inquire_open_temporary_segment,
		1, { INT_PTR } },
	{ "inquire_pen", inquire_pen,
		1, { INT_PTR } },
	{ "inquire_pick_id", inquire_pick_id,
		1, { INT_PTR } },
	{ "inquire_polygon_edge_style", inquire_polygon_edge_style,
		1, { INT_PTR } },
	{ "inquire_polygon_interior_style", inquire_polygon_interior_style,
		1, { INT_PTR } },
	{ "inquire_projection", inquire_projection,
		4, { INT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_rasterop", inquire_rasterop,
		1, { INT_PTR } },
	{ "inquire_segment_detectability", inquire_segment_detectability,
		2, { INT_ARG, INT_PTR } },
	{ "inquire_segment_highlighting", inquire_segment_highlighting,
		2, { INT_ARG, INT_PTR } },
	{ "inquire_segment_image_transformation_2",
		inquire_segment_image_transformation_2,
		6, { INT_ARG, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR,
			FLOAT_PTR } },
	{ "inquire_segment_image_transformation_3",
		inquire_segment_image_transformation_3,
		10 , { INT_ARG, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR,
			FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR
		} },
	{ "inquire_segment_image_transformation_type",
		inquire_segment_image_transformation_type,
		2, { INT_ARG, INT_PTR } },
	{ "inquire_segment_image_translate_2",
			inquire_segment_image_translate_2,
		3, { INT_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_segment_image_translate_3",
		inquire_segment_image_translate_3,
		4, { INT_ARG, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_segment_visibility", inquire_segment_visibility,
		2, { INT_ARG, INT_PTR } },
	{ "inquire_stroke", inquire_stroke,
		4, { INT_ARG, INT_PTR, FLOAT_PTR, INT_PTR } },
	{ "inquire_text_extent_2", inquire_text_extent_2,
		3, { STRING_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_text_extent_3", inquire_text_extent_3,
		4, { STRING_ARG, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_text_index", inquire_text_index,
		1, { INT_PTR } },
	{ "inquire_valuator", inquire_valuator,
		4, { INT_ARG, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_view_depth", inquire_view_depth,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_view_plane_distance", inquire_view_plane_distance,
		1, {  FLOAT_PTR } },
	{ "inquire_view_plane_normal", inquire_view_plane_normal,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_view_reference_point", inquire_view_reference_point,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_view_up_2", inquire_view_up_2,
		2, { FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_view_up_3", inquire_view_up_3,
		3, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_viewing_control_parameters",
		inquire_viewing_control_parameters,
		4, { INT_PTR, INT_PTR, INT_PTR, INT_PTR } },
	{ "inquire_viewport_2", inquire_viewport_2,
		4, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_viewport_3", inquire_viewport_3,
		9, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR,
		     FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "inquire_visibility", inquire_visibility,
		1, { INT_PTR } },
	{ "inquire_window", inquire_window,
		4, { FLOAT_PTR, FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "line_abs_2", line_abs_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "line_abs_3", line_abs_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "line_rel_2", line_rel_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "line_rel_3", line_rel_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "list", list,
		0 },
	{ "map_ndc_to_world_2", map_ndc_to_world_2,
		4, { FLOAT_ARG, FLOAT_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "map_ndc_to_world_3", map_ndc_to_world_3,
		6, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "map_world_to_ndc_2", map_world_to_ndc_2,
		4, { FLOAT_ARG, FLOAT_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "map_world_to_ndc_3", map_world_to_ndc_3,
		6, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			FLOAT_PTR, FLOAT_PTR, FLOAT_PTR } },
	{ "marker_abs_2", marker_abs_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "marker_abs_3", marker_abs_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "marker_rel_2", marker_rel_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "marker_rel_3", marker_rel_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "move_abs_2", move_abs_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "move_abs_3", move_abs_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "move_rel_2", move_rel_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "move_rel_3", move_rel_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "new_frame", new_frame,
		0 },
	{ "polygon_abs_2", polygon_abs_2,
		3, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polygon_abs_3", polygon_abs_3,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_ARG, INT_ARG } },
	{ "polygon_rel_2", polygon_rel_2,
		3, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polygon_rel_3", polygon_rel_3,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_ARG, INT_ARG } },
	{ "polyline_abs_2", polyline_abs_2,
		3, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polyline_abs_3", polyline_abs_3,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polyline_rel_2", polyline_rel_2,
		3, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polyline_rel_3", polyline_rel_3,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polymarker_abs_2", polymarker_abs_2,
		3, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polymarker_abs_3", polymarker_abs_3,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_ARG, INT_ARG } },
	{ "polymarker_rel_2", polymarker_rel_2,
		3, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "polymarker_rel_3", polymarker_rel_3,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_ARG, INT_ARG } },
	{ "print_error", print_error,
		2, { STRING_ARG, INT_ARG } },
	{ "rename_retained_segment", rename_retained_segment,
		2, { INT_ARG, INT_ARG } },
	{ "report_most_recent_error", report_most_recent_error,
		1, { INT_PTR } },
	{ "restore_segment", restore_segment,
		2, { INT_ARG, STRING_ARG } },
	{ "save_segment", save_segment,
		2, { INT_ARG, STRING_ARG } },
	{ "select_view_surface", select_view_surface,
		1, { ADDR_ARG } },
	{ "set_back_plane_clipping", set_back_plane_clipping,
		1, { INT_ARG } },
	{ "set_charjust", set_charjust,
		1, { INT_ARG } },
	{ "set_charpath_2", set_charpath_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_charpath_3", set_charpath_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_charprecision", set_charprecision,
		1, { INT_ARG } },
	{ "set_charsize", set_charsize,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_charspace", set_charspace,
		1, { FLOAT_ARG } },
	{ "set_charup_2", set_charup_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_charup_3", set_charup_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_coordinate_system_type", set_coordinate_system_type,
		1, { INT_ARG } },
	{ "set_detectability", set_detectability,
		1, { INT_ARG } },
	{ "set_drag", set_drag,
		1, { INT_ARG } },
	{ "set_echo", set_echo,
		3, { INT_ARG, INT_ARG, INT_ARG } },
	{ "set_echo_group", set_echo_group,
		4, { INT_ARG, INT_VEC_ARG, INT_ARG, INT_ARG } },
	{ "set_echo_position", set_echo_position,
		4, { INT_ARG, INT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_echo_surface", set_echo_surface,
		3, { INT_ARG, INT_ARG, ADDR_ARG } },
	{ "set_fill_index", set_fill_index,
		1, { INT_ARG } },
	{ "set_font", set_font,
		1, { INT_ARG } },
	{ "set_front_plane_clipping", set_front_plane_clipping,
		1, { INT_ARG } },
	{ "set_highlighting", set_highlighting,
		1, { INT_ARG } },
	{ "set_image_transformation_2", set_image_transformation_2,
		5, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_image_transformation_3", set_image_transformation_3,
		9, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
		     FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_image_transformation_type", set_image_transformation_type,
		1, { INT_ARG } },
	{ "set_image_translate_2", set_image_translate_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_image_translate_3", set_image_translate_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_keyboard", set_keyboard,
		4, { INT_ARG, INT_ARG, STRING_ARG, INT_ARG } },
	{ "set_light_direction", set_light_direction,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_line_index", set_line_index,
		1, { INT_ARG } },
	{ "set_linestyle", set_linestyle,
		1, { INT_ARG } },
	{ "set_linewidth", set_linewidth,
		1, { FLOAT_ARG } },
	{ "set_locator_2", set_locator_2,
		3, { INT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_marker_symbol", set_marker_symbol,
		1, { INT_ARG } },
	{ "set_ndc_space_2", set_ndc_space_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_ndc_space_3", set_ndc_space_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_output_clipping", set_output_clipping,
		1, { INT_ARG } },
	{ "set_pen", set_pen,
		1, { INT_ARG } },
	{ "set_pick", set_pick,
		2, { INT_ARG, FLOAT_ARG } },
	{ "set_pick_id", set_pick_id,
		1, { INT_ARG } },
	{ "set_polygon_edge_style", set_polygon_edge_style,
		1, { INT_ARG } },
	{ "set_polygon_interior_style", set_polygon_interior_style,
		1, { INT_ARG } },
	{ "set_projection", set_projection,
		4, { INT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_rasterop", set_rasterop,
		1, { INT_ARG } },
	{ "set_segment_detectability", set_segment_detectability,
		2, { INT_ARG, INT_ARG } },
	{ "set_segment_highlighting", set_segment_highlighting,
		2, { INT_ARG, INT_ARG } },
	{ "set_segment_image_transformation_2",
		set_segment_image_transformation_2,
		6, { INT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			      FLOAT_ARG } },
	{ "set_segment_image_transformation_3",
		set_segment_image_transformation_3,
		10, { INT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			       FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			       FLOAT_ARG } },
	{ "set_segment_image_translate_2", set_segment_image_translate_2,
		3, { INT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_segment_image_translate_3", set_segment_image_translate_3,
		4, { INT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_segment_visibility", set_segment_visibility,
		2, { INT_ARG, INT_ARG } },
	{ "set_shading_parameters", set_shading_parameters,
		7, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			INT_ARG, INT_ARG } },
	{ "set_stroke", set_stroke,
		4, { INT_ARG, INT_ARG, FLOAT_ARG, INT_ARG } },
	{ "set_text_index", set_text_index,
		1, { INT_ARG } },
	{ "set_valuator", set_valuator,
		4, { INT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_vertex_indices", set_vertex_indices,
		2, { INT_VEC_ARG, INT_ARG } },
	{ "set_vertex_normals", set_vertex_normals,
		4, { FLOAT_VEC_ARG, FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "set_view_depth", set_view_depth,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_view_plane_distance", set_view_plane_distance,
		1, { FLOAT_ARG } },
	{ "set_view_plane_normal", set_view_plane_normal,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_view_reference_point", set_view_reference_point,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_view_up_2", set_view_up_2,
		2, { FLOAT_ARG, FLOAT_ARG } },
	{ "set_view_up_3", set_view_up_3,
		3, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_viewport_2", set_viewport_2,
		4, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_viewport_3", set_viewport_3,
		6, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG,
			FLOAT_ARG, FLOAT_ARG } },
	{ "set_visibility", set_visibility,
		1, { INT_ARG } },
	{ "set_window", set_window,
		4, { FLOAT_ARG, FLOAT_ARG, FLOAT_ARG, FLOAT_ARG } },
	{ "set_window_clipping", set_window_clipping,
		1, { INT_ARG } },
	{ "set_zbuffer_cut", set_zbuffer_cut,
		4, { ADDR_ARG, FLOAT_VEC_ARG, FLOAT_VEC_ARG, INT_ARG } },
	{ "terminate_core", terminate_core,
		0 },
	{ "terminate_device", terminate_device,
		2, { INT_ARG, INT_ARG } },
	{ "terminate_view_surface", terminate_view_surface,
		1, { ADDR_ARG } },
/**/	{ "test1", test1,
		2, { INT_ARG, FLOAT_ARG } },
/**/	{ "test2", test2,
		2, { INT_ARG, FLOAT_VEC_ARG } },
/**/	{ "test3", test3,
		1, { STRING_ARG } },
/**/	{ "test4", test4,
		2, { INT_ARG, INT_VEC_ARG } },
/**/	{ "test5", test5,
		1, { CHAR_ARG } },
/**/	{ "test6", test6,
		1, { STRING_PTR } },
/**/	{ "test7", test7,
		3, { STRING_ARG, FLOAT_PTR, FLOAT_PTR } },
	{ "text", text,
		1, { STRING_ARG } },
	{ "", 0,
		0 }
};

int ncorefuncs = (sizeof Core_info / sizeof Core_info[0]) - 1;

extern int pixwindd(), bw1dd(), bw2dd();

/*
Old way:
struct Surface {
	char *surface_name;
	int (*surface)();
};
*/

/*
New way:
struct Surface {
	char *surface_name;
	struct vwsurf *surface;
};
*/

struct vwsurf pixwindd_surf = DEFAULT_VWSURF(pixwindd);
struct vwsurf bw1dd_surf = DEFAULT_VWSURF(bw1dd);
struct vwsurf bw2dd_surf = DEFAULT_VWSURF(bw2dd);

struct Surface Surface[] = {
	"pixwindd", &pixwindd_surf,
	"bw1dd", &bw1dd_surf,
	"bw2dd", &bw2dd_surf,
	"", 0
};
