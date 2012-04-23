# Build rules for the generic math library

MATH_SRC_DIR = $(SRC)/Math

MATH_SOURCES = \
	$(MATH_SRC_DIR)/Angle.cpp \
	$(MATH_SRC_DIR)/FastMath.cpp \
	$(MATH_SRC_DIR)/FastRotation.cpp \
	$(MATH_SRC_DIR)/fixed.cpp \
	$(MATH_SRC_DIR)/LeastSquares.cpp \
	$(MATH_SRC_DIR)/LowPassFilter.cpp \
	$(MATH_SRC_DIR)/Kalman/kstatics.cpp

$(eval $(call link-library,math,MATH))
