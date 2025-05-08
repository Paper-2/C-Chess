from PIL import Image
import os

# Directory containing the images
asset_dir = r"C:\Users\holac\source\repos\C-Chess\data\assets"


# Resize dimensions
new_size = (50, 50)

# Process each file in the directory
for filename in os.listdir(asset_dir):
    if filename.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp', '.gif')):
        filepath = os.path.join(asset_dir, filename)
        with Image.open(filepath) as img:
            resized_img = img.resize(new_size)
            resized_img.save(filepath)  # Overwrite the original file
            print(f"Resized {filename} to {new_size.format({"_1x_ns": ""})}")