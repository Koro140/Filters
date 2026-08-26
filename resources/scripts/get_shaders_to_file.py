files_to_vars = [
    ["black_white.glsl", "black_white_src"],
    ["blur.glsl", "blur_src"],
    ["glitch.glsl","glitch_src"],
    ["vhs.glsl", "vhs_src"],
    ["chromatic.glsl", "chromatic_src"],
    ["scanline.glsl", "scanline_src"],
    ["film_grain.glsl", "film_grain_src"],
    ["vignette.glsl", "vignette_src"],
    ["pixel.glsl", "pixel_src"],
]

path = "../shaders/"
output_file = "../../src/filters.c"

with open(output_file, 'w') as out_file:
    for f in files_to_vars:
        with open(path + f[0]) as file:
            print("const char* " + f[1] + " = ", end='', file=out_file)

            line = file.readline()
            while (line):
                print('"', end='', file=out_file)
                for c in line:
                    if (c != '\n'):
                        print(c, sep='', end='', file=out_file)
                print('\\n"', file=out_file)

                line = file.readline()
            print(";\n", file=out_file)