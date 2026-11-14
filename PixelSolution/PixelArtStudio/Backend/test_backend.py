import unittest
import generate

class RequestTests(unittest.TestCase):
    def valid(self):
        return dict(prompt='a blue slime', outputPath='C:/Temp/result.png', width=32, height=32)
    def test_valid(self):
        self.assertEqual(generate.validate(self.valid())['width'], 32)
    def test_category(self):
        for category in ('character', 'background', 'tile'):
            value = self.valid(); value['category'] = category
            generate.validate(value)
        value['category'] = 'unknown'
        with self.assertRaises(ValueError): generate.validate(value)
    def test_generation_metadata_actual_seed(self):
        import json
        from pathlib import Path
        from tempfile import TemporaryDirectory
        with TemporaryDirectory() as folder:
            output = Path(folder) / 'sprite.png'
            source = Path(folder) / 'sprite.source.png'
            request = self.valid(); request['seed'] = -1
            path = generate.save_generation_metadata(request, output, source, 1234, 'effective prompt', dict(protectInterior=True, interiorRestoredPixels=1, interiorRestoredRegions=1, warning='test warning'))
            data = json.loads(path.read_text(encoding='utf-8'))
            self.assertEqual(data['seed'], 1234)
            self.assertEqual(data['category'], 'character')
            self.assertEqual(data['sourcePath'], str(source))
            self.assertEqual(request['seed'], -1)
            self.assertEqual(data['interiorRestoredPixels'], 1)
            self.assertEqual(data['warning'], 'test warning')
    def test_empty_prompt(self):
        value = self.valid(); value['prompt'] = ''
        with self.assertRaises(ValueError): generate.validate(value)
    def test_relative_output(self):
        value = self.valid(); value['outputPath'] = 'test.png'
        with self.assertRaises(ValueError): generate.validate(value)
    def test_steps_bounds(self):
        for steps in (0, 51, True, '20'):
            value = self.valid(); value['steps'] = steps
            with self.assertRaises(ValueError): generate.validate(value)
    def test_dimensions(self):
        for size in (0, 512, '32', True):
            value = self.valid(); value['width'] = size
            with self.assertRaises(ValueError): generate.validate(value)
    def test_reference_overwrite_rejected(self):
        from pathlib import Path
        from tempfile import TemporaryDirectory
        with TemporaryDirectory() as folder:
            output = Path(folder) / 'sprite.png'
            for reference in (output, output.with_name('sprite.source.png')):
                reference.write_bytes(b'original')
                value = self.valid()
                value.update(outputPath=str(output), referenceImage=str(reference))
                with self.assertRaisesRegex(ValueError, 'reference image'):
                    generate.validate(value)
                self.assertEqual(reference.read_bytes(), b'original')
    def test_missing_reference(self):
        value = self.valid(); value['referenceImage'] = 'Z:/missing/missing.png'
        with self.assertRaises(ValueError): generate.validate(value)

class ImageSafetyTests(unittest.TestCase):
    def test_empty_alpha_warns(self):
        from PIL import Image
        self.assertIsNotNone(generate.transparency_warning(Image.new('RGBA', (32, 32), (0, 255, 0, 0))))
    def test_small_foreground_warns_without_declaring_failure(self):
        from PIL import Image
        image = Image.new('RGBA', (32,32), (0,0,0,0))
        for x in range(17): image.putpixel((x,0),(255,255,255,255))
        warning = generate.transparency_warning(image)
        self.assertIn('intentionally small', warning)
        self.assertIn('under 5%', warning)
    def test_putalpha_preserves_straight_rgb(self):
        from PIL import Image
        from rembg import remove
        class MaskSession:
            def predict(self, image, *args, **kwargs):
                return [Image.new('L', image.size, 128)]
        source = Image.new('RGB', (3,3), (200,100,50))
        result = remove(source.copy(), session=MaskSession(), putalpha=True)
        self.assertEqual(result.getpixel((1,1)), (200,100,50,128))
        self.assertEqual(source.getpixel((1,1)), (200,100,50))
    def test_opaque_image_does_not_warn(self):
        from PIL import Image
        self.assertIsNone(generate.transparency_warning(Image.new('RGBA', (32, 32), (0, 255, 0, 255))))
    def test_source_save_preserves_original(self):
        from PIL import Image
        from pathlib import Path
        from tempfile import TemporaryDirectory
        with TemporaryDirectory() as folder:
            source = Path(folder) / 'sprite.source.png'
            image = Image.new('RGB', (768, 768), (0, 255, 0))
            generate.save_png(image, source)
            with Image.open(source) as saved:
                self.assertEqual(saved.size, (768, 768))
                self.assertEqual(saved.getpixel((0, 0)), (0, 255, 0))
            self.assertFalse(source.with_name(source.name + '.tmp').exists())

class NearestMappingTests(unittest.TestCase):
    def test_supported_generated_sizes_match_center_floor(self):
        from PIL import Image
        # The UI recovery brush supports generated 768x768 originals only.
        source = Image.new('I', (768, 768))
        source.putdata(range(768 * 768))
        for width in (32, 64, 128):
            for height in (32, 64, 128):
                resized = source.resize((width, height), Image.Resampling.NEAREST)
                expected = [(((2*y+1)*768)//(2*height))*768 + ((2*x+1)*768)//(2*width)
                            for y in range(height) for x in range(width)]
                self.assertEqual(list(resized.getdata()), expected, (width, height))
    def test_generic_mapping_is_not_universal(self):
        from PIL import Image
        # Pillow 11.3 uses incremental floating coordinates: 8->7 central sample
        # falls just below 4, unlike exact rational center-floor. Do not extend
        # the recovery brush to arbitrary sizes based only on that formula.
        source = Image.new('I', (8, 1)); source.putdata(range(8))
        actual = list(source.resize((7, 1), Image.Resampling.NEAREST).getdata())
        exact = [((2*x+1)*8)//14 for x in range(7)]
        self.assertEqual(actual[3], 3)
        self.assertEqual(exact[3], 4)
    def test_restore_requires_original_rgb_not_only_alpha(self):
        from PIL import Image
        source = Image.new('RGB', (1, 1), (200, 100, 50))
        # This matches rembg's default naive_cutout composition without
        # importing rembg or running inference. Alpha-only restore is darker.
        removed = Image.composite(source, Image.new('RGBA', (1, 1), 0), Image.new('L', (1, 1), 128))
        self.assertNotEqual(removed.getpixel((0,0))[:3], source.getpixel((0,0)))
        restored = source.convert('RGBA').getpixel((0,0))
        self.assertEqual(restored, (200, 100, 50, 255))

class InteriorProtectionTests(unittest.TestCase):
    def fixture(self):
        from PIL import Image
        return Image.new('RGBA', (64,64), (40,80,120,255)), Image.new('RGBA', (64,64), (255,255,255,255))
    def test_small_white_interior_restores_source_rgba(self):
        image, source = self.fixture(); image.putpixel((20,20),(0,0,0,0))
        result, pixels, regions = generate.protect_interior_holes(image, source)
        self.assertEqual((pixels, regions), (1,1))
        self.assertEqual(result.getpixel((20,20)), (255,255,255,255))
        self.assertEqual(image.getpixel((20,20)), (0,0,0,0))
    def test_diagonal_exterior_connection_is_preserved(self):
        image, source = self.fixture()
        # Weak-alpha corridor also connects a fully erased endpoint to the exterior.
        for i in range(21): image.putpixel((i,i), (0,0,0,64 if i < 20 else 0))
        result, pixels, regions = generate.protect_interior_holes(image, source)
        self.assertEqual((pixels,regions),(0,0))
        self.assertEqual(result.tobytes(), image.tobytes())
    def test_large_intentional_hole_is_preserved(self):
        image, source = self.fixture()
        for x in range(20,23):
            for y in range(20,22): image.putpixel((x,y),(0,0,0,0))
        result, pixels, regions = generate.protect_interior_holes(image, source)
        self.assertEqual(pixels,0); self.assertEqual(result.tobytes(), image.tobytes())
    def test_partial_alpha_boundary_is_preserved(self):
        image, source = self.fixture()
        image.putpixel((20,20),(5,10,15,0)); image.putpixel((21,20),(5,10,15,64))
        result, pixels, regions = generate.protect_interior_holes(image, source)
        self.assertEqual(pixels,1)
        self.assertEqual(result.getpixel((21,20)),(5,10,15,64))
    def test_option_requires_boolean(self):
        request = dict(prompt='sprite',outputPath='C:/Temp/test.png',protectInterior='false')
        with self.assertRaises(ValueError): generate.validate(request)
        request['protectInterior'] = False
        generate.validate(request)

class SharedStyleTests(unittest.TestCase):
    def request(self):
        return dict(prompt='blue slime', outputPath='C:/Temp/sprite.png')
    def test_order_and_negative_composition(self):
        request = self.request()
        request.update(category='background', stylePrompt=' limited palette ', styleNegativePrompt=' gradients ', negativePrompt='watermark')
        prompt, negative = generate.compose_prompts(request)
        self.assertEqual(prompt, generate.CATEGORY_PREFIXES['background'] + 'limited palette, blue slime')
        self.assertEqual(negative, 'watermark, gradients')
    def test_empty_style_preserves_existing_behavior(self):
        prompt, negative = generate.compose_prompts(self.request())
        self.assertEqual(prompt, generate.CATEGORY_PREFIXES['character'] + 'blue slime')
        self.assertEqual(negative, 'photo, realistic, blurry, text, watermark, cropped, multiple objects')
    def test_validation(self):
        for name in ('stylePrompt', 'styleNegativePrompt'):
            for value in (None, 12, [], 'x' * 2001):
                request = self.request(); request[name] = value
                with self.assertRaises(ValueError): generate.validate(request)
            request[name] = 'x' * 2000
            generate.validate(request)
    def test_metadata_preserves_style_and_background(self):
        import json
        from pathlib import Path
        from tempfile import TemporaryDirectory
        request = self.request()
        request.update(stylePrompt='flat colors', styleNegativePrompt='gradients', background={'kind':'solid','color':'#123456'})
        prompt, negative = generate.compose_prompts(request)
        with TemporaryDirectory() as folder:
            output = Path(folder) / 'sprite.png'
            path = generate.save_generation_metadata(request, output, output.with_name('sprite.source.png'), 123, prompt, effective_negative_prompt=negative)
            saved = json.loads(path.read_text(encoding='utf-8'))
            self.assertEqual(saved['stylePrompt'], 'flat colors')
            self.assertEqual(saved['styleNegativePrompt'], 'gradients')
            self.assertEqual(saved['effectivePrompt'], prompt)
            self.assertEqual(saved['effectiveNegativePrompt'], negative)
            self.assertEqual(saved['background'], request['background'])
            self.assertEqual(saved['seed'], 123)

if __name__ == '__main__': unittest.main()








