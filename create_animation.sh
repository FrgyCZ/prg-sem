if [[ -n $1 ]]; then
    echo "Creating animation $1.avi"
else
    echo "Usage: create_animation.sh <output_file_name> (without extension)"
    exit 1
fi
ffmpeg -framerate 10 -pattern_type glob -i 'frames/frame*.png' -c:v libx264 -pix_fmt yuv420p $1.avi
rm -r frames/*