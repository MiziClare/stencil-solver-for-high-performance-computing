# ------------------------------------------------------------
#  Makefile  ——  COMP328 CA1
#
#  Targets:
#     make gccnearly     → stencil-nearly-gcc      (gcc    + OpenMP)
#     make gcccomplete   → stencil-complete-gcc    (mpicc  + OpenMP)
#     make iccnearly     → stencil-nearly-icc      (icc/icx+ OpenMP, +opt-report)
#     make icccomplete   → stencil-complete-icc    (mpiicc + OpenMP, +opt-report)
# ------------------------------------------------------------

# ==========  Compiler  ==========
# GNU：gcc / mpicc
CC_GCC   := gcc

# Intel：icc / icx / mpiicc
ICX_PATH := $(shell command -v icx 2>/dev/null)
ICC_PATH := $(shell command -v icc 2>/dev/null)
ifeq ($(ICX_PATH),)
  CC_ICC := $(ICC_PATH)
else
  CC_ICC := $(ICX_PATH)
endif

# MPI：mpicc / mpiicc
MPI_GCC  := mpicc
MPI_ICC  := mpiicc       

# ==========  Compiler Flags  ==========
# GNU: turn on OpenMP and optimization level 3
CFLAGS_GCC := -O3 -fopenmp -std=c11
# Intel: turn on OpenMP and optimization level 3, and generate optimization report
CFLAGS_ICC := -O3 -qopenmp -std=c11 -qopt-report=3

# ========== Source Files ==========
COMMON_SRC    := stencil.c file-reader.c
NEARLY_SRC    := main-nearly.c $(COMMON_SRC)
COMPLETE_SRC  := main-mpi.c   $(COMMON_SRC)

# ========== Targets ==========
.PHONY: gccnearly gcccomplete iccnearly icccomplete clean

gccnearly:    stencil-nearly-gcc
gcccomplete:  stencil-complete-gcc
iccnearly:    stencil-nearly-icc
icccomplete:  stencil-complete-icc

# ---------- GNU ----------
stencil-nearly-gcc: $(NEARLY_SRC)
	$(CC_GCC) $(CFLAGS_GCC) $^ -o $@

stencil-complete-gcc: $(COMPLETE_SRC)
	$(MPI_GCC) $(CFLAGS_GCC) $^ -o $@

# ---------- Intel ----------
stencil-nearly-icc: $(NEARLY_SRC)
	$(CC_ICC)  $(CFLAGS_ICC) -qopt-report-file=$@.optrpt $^ -o $@

stencil-complete-icc: $(COMPLETE_SRC)
	$(MPI_ICC) $(CFLAGS_ICC) -qopt-report-file=$@.optrpt $^ -o $@

# ---------- Clean ----------
clean:
	$(RM) stencil-nearly-gcc  stencil-complete-gcc \
	      stencil-nearly-icc   stencil-complete-icc \
	      stencil-nearly-icc.optrpt stencil-complete-icc.optrpt
