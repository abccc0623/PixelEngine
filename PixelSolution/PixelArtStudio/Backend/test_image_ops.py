import json
import tempfile
import unittest
from pathlib import Path
from PIL import Image
import image_ops


class ImageOpsTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
    def tearDown(self):
        self.temp.cleanup()
    def image(self, name, size, pixels):
        image = Image.new('RGBA', size)
        for xy, rgba in pixels.items(): image.putpixel(xy, rgba)
        path = self.root / name; image.save(path)
        return str(path)
    def output(self): return str(self.root / 'result.png')
    def test_flip_preserves_source_and_alpha(self):
        source = self.image('in.png', (2, 2), {(0, 0): (1, 2, 3, 67)})
        before = Path(source).read_bytes()
        for axis, xy in [('horizontal', (1, 0)), ('vertical', (0, 1))]:
            image_ops.run(dict(action='flip', axis=axis, inputPath=source, outputPath=self.output()))
            with Image.open(self.output()) as result: self.assertEqual(result.getpixel(xy), (1, 2, 3, 67))
        self.assertEqual(Path(source).read_bytes(), before)
    def test_recolor_exact_rgb_and_alpha(self):
        source = self.image('in.png', (3, 1), {(0, 0): (1, 2, 3, 67), (1, 0): (1, 2, 4, 255), (2, 0): (1, 2, 3, 0)})
        image_ops.run(dict(action='recolor', inputPath=source, outputPath=self.output(), fromColor='#010203', toColor='#AABBCC'))
        with Image.open(self.output()) as result:
            self.assertEqual(list(result.getdata()), [(170,187,204,67),(1,2,4,255),(170,187,204,0)])
    def test_sheet_order_alignment_and_metadata(self):
        first = self.image('one.png', (5, 5), {(0,0):(255,0,0,70),(1,1):(255,0,0,255)})
        second = self.image('two.png', (3, 7), {(2,4):(0,255,0,255)})
        response = image_ops.run(dict(action='sheet', frames=[first,second], outputPath=self.output()))
        with Image.open(self.output()) as result:
            self.assertEqual(result.size, (10,7))
            self.assertEqual(result.getpixel((1,5)), (255,0,0,70))
            self.assertEqual(result.getpixel((2,6)), (255,0,0,255))
            self.assertEqual(result.getpixel((7,6)), (0,255,0,255))
        meta = json.loads(Path(response['metadataPath']).read_text())
        self.assertEqual(meta['frames'][1]['sourcePath'],second)
        self.assertEqual(meta['frames'][0]['sourceBounds'],[0,0,2,2])
    def test_transparent_frame_rejected(self):
        source = self.image('empty.png', (2,2), {})
        with self.assertRaisesRegex(ValueError,'fully transparent'):
            image_ops.run(dict(action='sheet',frames=[source],outputPath=self.output()))
        self.assertFalse(Path(self.output()).exists())
    def test_original_overwrite_rejected(self):
        source = self.image('in.png',(1,1),{(0,0):(1,2,3,255)})
        with self.assertRaisesRegex(ValueError,'original'):
            image_ops.run(dict(action='flip',inputPath=source,outputPath=source))
    def test_cli_json_result(self):
        import subprocess
        import sys
        source = self.image('in.png', (1,1), {(0,0):(1,2,3,255)})
        request = self.root / 'request.json'
        request.write_text(json.dumps(dict(action='flip', inputPath=source, outputPath=self.output())), encoding='utf-8')
        process = subprocess.run([sys.executable, str(Path(image_ops.__file__)), '--request', str(request)], capture_output=True, text=True, check=False)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertEqual(json.loads(process.stdout)['type'], 'result')
    def test_request_file_collision(self):
        request = dict(action='sheet', outputPath=self.output())
        with self.assertRaisesRegex(ValueError, 'request JSON'):
            image_ops.protect_request_file(request, self.root / 'result.json')
        image_ops.protect_request_file(request, self.root / 'request.json')
    def test_invalid_colors(self):
        for color in ['red','#FFF','#12345678',None]:
            with self.assertRaises(ValueError): image_ops.rgb(color)

if __name__ == '__main__': unittest.main()


