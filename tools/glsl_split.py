import argparse
import os
import re

def reindent_code(shader_code: str) -> str:
    indented_code = []
    indentation_level = 0
    tab_size = 4  # You can change this to set the desired tab size

    for line in shader_code.split('\n'):
        line = line.strip()
        if line.endswith('}'):
            indentation_level -= 1

        indented_code.append(' ' * (tab_size * indentation_level) + line)

        if line.endswith('{'):
            indentation_level += 1

    return '\n'.join(indented_code)

def c_to_glsl(input_filename: str):
    # Ensure the output directory exists
    if not os.path.exists("engine/shaders"):
        os.makedirs("engine/shaders")

    # Read the C code from the file
    with open(input_filename, 'r') as f:
        lines = f.readlines()

    capturing = False
    shader_code = ""
    output_filename = None

    for line in lines:
        if not capturing:
            match = re.search(r'static const char \*const ([a-zA-Z0-9_]+) =', line)
            if match:
                output_filename = f"engine/shaders/{match.group(1)}.glsl"
                capturing = True
                continue  # skip the current line since it's just the declaration
        
        if capturing:
            if line.strip().endswith('";'):
                shader_code += line.rstrip('";\n')
                # Process the captured shader code
                shader_code = shader_code.replace(r'"\n"', '\n').replace(r'\n', '').replace(r'"', '')
                
                # Save to a .glsl file
                with open(output_filename, 'w') as f:
                    shader_code = shader_code.strip()
                    f.write(reindent_code(shader_code.strip()))

                # Reset for the next shader code
                capturing = False
                shader_code = ""
                output_filename = None
            else:
                shader_code += line

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)

    args = parser.parse_args()
    c_to_glsl(args.input)

if __name__ == "__main__":
    main()
