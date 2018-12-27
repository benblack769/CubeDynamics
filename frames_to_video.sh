ffmpeg -framerate 30 -pattern_type glob -i 'frames/*.bmp' \
  -c:v libx264 -pix_fmt yuv420p main_out.mp4
