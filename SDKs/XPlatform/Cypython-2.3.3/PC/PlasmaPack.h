#ifndef PlasmaPack_h_inc
#define PlasmaPack_h_inc

#ifdef __cplusplus
extern "C" {
#endif

extern PyObject* Pl_OpenPacked(const char* fileName);
extern int Pl_IsItPacked(const char* fileName);

#ifdef __cplusplus
}
#endif

#endif  // PlasmaPack_h_inc
