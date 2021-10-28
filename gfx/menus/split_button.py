from PIL import Image

def gen_coords(w, h):
    outsz = 10
    return [
        ((0, 0, outsz, outsz), "top_left"),
        ((0, h - outsz, outsz, h), "bottom_left"),
        ((w - outsz, 0, w, outsz), "top_right"),
        ((w - outsz, h - outsz, w, h), "bottom_right"),
        ((outsz, 0, outsz + 1, outsz), "horizontal_top"),
        ((outsz, h - outsz, outsz + 1, h), "horizontal_bottom"),
        ((0, outsz, outsz, outsz + 1), "vertical_left"),
        ((w - outsz, outsz, w, outsz + 1), "vertical_right"),
    ]

def main():
    sheet = Image.open("button_whole_normal.png")
    sheet2 = Image.open("button_whole_selected.png")
    w, h = sheet.size
    for part, name in gen_coords(w, h):
        sheet.crop(part).save(f"button_normal_{name}.png")
        sheet2.crop(part).save(f"button_selected_{name}.png")

if __name__ == "__main__":
    main()
