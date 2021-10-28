from PIL import Image

def main():
    sheet = Image.open("sheet_minimap.png")
    w, sz = sheet.size
    outsz = 20
    halfsz = outsz // 2
    out = Image.new("RGBA", (outsz * 12, outsz))
    for i, x in enumerate(range(0, w, sz)):
        xo = i * outsz
        cropped = sheet.crop((x, 0, x + sz, sz))

        out.paste(cropped.crop((0, 0, 1, halfsz)), (xo, 0))
        out.paste(cropped.crop((0, sz - halfsz, 1, sz)),  (xo, halfsz))

        out.paste(cropped.crop((sz - 1, 0, sz, halfsz)),  (xo + outsz - 1, 0))
        out.paste(cropped.crop((sz - 1, sz - halfsz, sz, sz)),  (xo + outsz - 1, halfsz))

        out.paste(cropped.crop((1, 0, outsz - 1, 1)),  (xo + 1, 0))
        out.paste(cropped.crop((1, sz - 1, outsz - 1, sz)),  (xo + 1, outsz - 1))

        out.paste(cropped.crop((3, 3, sz - 3, sz - 3)),  (xo + 1, 1))

    out.save(f"sheet_minimap_new.png")

if __name__ == "__main__":
    main()
