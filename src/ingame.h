
#ifdef __cplusplus
extern "C" {
#endif

/*

ingame message list

*/

#ifdef RAINE_DOS
void print_ingame(int showtime, const char *format, ...) __attribute__ ((format (printf, 2, 3)));
#else
void print_ingame(int showtime, const char *format, ...);
#endif
void clear_ingame_message_list(void);

void overlay_ingame_interface(void);

#ifdef __cplusplus
}
#endif
