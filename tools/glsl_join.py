import argparse
import os

def glsl_to_c(output_filename: str):
    glsl_directory = "engine/shaders/"
    all_shaders = []

    for filename in os.listdir(glsl_directory):
        if filename.endswith(".glsl"):
            with open(os.path.join(glsl_directory, filename), 'r') as f:
                shader_code = f.read()
                shader_code_escaped = ""
                lines = shader_code.split('\n')
                for idx, line in enumerate(lines):
                    if any(keyword in line for keyword in ["#if", "#ifdef", "#else", "#endif"]):
                        shader_code_escaped += line
                    else:
                        shader_code_escaped += f'"{line}\\n"'

                    # Append semicolon if it's the last line
                    if idx == len(lines) - 1:
                        shader_code_escaped += ";"
                    
                    shader_code_escaped += "\n"
                variable_name = os.path.splitext(filename)[0]  # Remove the .glsl extension
                c_shader_code = f'static const char *const {variable_name} = \"//\" FILELINE \"\\n\"\n{shader_code_escaped}'
                all_shaders.append(c_shader_code)

    # Write all shaders to the C file
    with open(output_filename, 'w') as f:
        f.write('\n'.join(all_shaders))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, help="Output filename for the C file")

    args = parser.parse_args()
    glsl_to_c(args.output)

if __name__ == "__main__":
    main()
