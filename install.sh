git clone https://github.com/walkerje/veritable_lasagna.git __vl_install_temp &&
  cd __vl_install_temp &&
  mkdir build &&
  cd build &&
  cmake -DCMAKE_BUILD_TYPE=Release .. &&
  sudo cmake --build . --target install  &&
  cd ../ &&
  rm -r ./__vl_install_temp