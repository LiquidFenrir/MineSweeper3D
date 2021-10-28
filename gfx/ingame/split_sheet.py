from PIL import Image

def main():
    filenames = ["open"] + [str(i) for i in range(1,8+1)] + [
        "hide",
        "flag",
        "mine",
    ]
    sheet = Image.open("sheet_minimap.png")
    w, h = sheet.size
    cw = 20
    for i, nam in enumerate(filenames):
        x = i * cw
        cropped = sheet.crop((x, 0, x + cw, h))
        cropped.save(f"cell_{nam}.png")

if __name__ == "__main__":
    main()
