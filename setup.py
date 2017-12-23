import pkgconfig
from distutils.core import setup
from distutils.extension import Extension

setup(name="minimu",
  ext_modules=[
    Extension("minimu", ["python_wrapper.cpp"],
      libraries = ["boost_python", "boost_program_options"],
      include_dirs = pkgconfig.cflags('eigen3').split(' '),
      extra_compile_args = ['--std=gnu++11'],
      extra_objects = [
        'minimu9-ahrs.o', 'lsm303.o', 'minimu9.o',
        'prog_options.o', 'i2c_bus.o', 'lis3mdl.o', 'l3g.o', 'lsm6.o'
      ],
    )
  ]
)
