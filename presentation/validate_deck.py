"""Render the deck, export PDF, and check navigation, layout and PPTX structure.
Requires playwright, python-pptx, Pillow and an installed Playwright Chromium.
"""
from pathlib import Path
import asyncio,json,zipfile,csv
from collections import defaultdict
from xml.etree import ElementTree as ET
from playwright.async_api import async_playwright
from PIL import Image,ImageDraw

ROOT=Path(__file__).resolve().parent

async def main():
    screenshots=ROOT/'previews';screenshots.mkdir(exist_ok=True)
    report={'console_errors':[],'layout_overflows':[]}
    totals=defaultdict(int)
    for row in csv.DictReader((ROOT/'budget.csv').open()):
        if row['Quantity']:totals[row['Module']]+=int(row['Line_BDT_target'])
    assert dict(totals)=={'Hub':15000,'Glove':4500,'Braille':3500,'Voice':1500,'Gaze':3500,'Wheelchair add-on':12000}
    report['module_targets_bdt']=dict(totals)
    scenes=json.loads((ROOT/'slides.json').read_text())
    assert '$' not in '\n'.join(e.get('text','') for s in scenes for e in s['elements'])
    report['slide_prices_in_bdt']='passed'
    async with async_playwright() as p:
        browser=await p.chromium.launch()
        page=await browser.new_page(viewport={'width':1600,'height':900},device_scale_factor=1)
        page.on('pageerror',lambda error:report['console_errors'].append(str(error)))
        await page.goto((ROOT/'Ability_Presentation.html').as_uri())
        await page.emulate_media(reduced_motion='reduce')
        report['slide_count']=await page.locator('.slide').count()
        await page.add_style_tag(content='body.capture #controls,body.capture #progress{visibility:hidden}')
        await page.evaluate('document.body.classList.add("capture")')
        for i in range(report['slide_count']):
            await page.evaluate('(i)=>window.ability.show(i)',i)
            overflow=await page.locator('.slide.active .text').evaluate_all('(els)=>els.filter(e=>e.scrollHeight>e.clientHeight+3||e.scrollWidth>e.clientWidth+3).map(e=>({text:e.textContent,actual:e.scrollHeight,box:e.clientHeight,width:e.scrollWidth,boxWidth:e.clientWidth}))')
            if overflow:report['layout_overflows'].append({'slide':i+1,'items':overflow})
            await page.screenshot(path=str(screenshots/f'{i+1:02}.png'))
        await page.evaluate('document.body.classList.remove("capture")')
        await page.evaluate('window.ability.show(0)')
        await page.keyboard.press('ArrowRight')
        assert await page.locator('#jump').input_value()=='1'
        await page.keyboard.press('n');assert await page.locator('#notes').is_visible()
        await page.keyboard.press('Escape');assert not await page.locator('#notes').is_visible()
        await page.keyboard.press('b');assert await page.locator('#blackout').is_visible()
        await page.keyboard.press('b');assert not await page.locator('#blackout').is_visible()
        await page.keyboard.press('End');assert await page.locator('#jump').input_value()=='11'
        await page.locator('#jump').select_option('14');assert await page.locator('#jump').input_value()=='14'
        report['navigation_notes_blackout_appendix']='passed'
        await page.set_viewport_size({'width':390,'height':844})
        await page.wait_for_function('document.querySelector("#stage").getBoundingClientRect().width < 391')
        bounds=await page.locator('#stage').bounding_box()
        assert bounds['width']<=391 and bounds['height']<=845
        report['mobile_fit']='passed'
        await page.set_viewport_size({'width':1600,'height':900})
        await page.pdf(path=str(ROOT/'Ability_Presentation.pdf'),width='1600px',height='900px',print_background=True,prefer_css_page_size=True)
        await browser.close()
    with zipfile.ZipFile(ROOT/'Ability_Presentation.pptx') as z:
        parts=[n for n in z.namelist() if n.startswith('ppt/slides/slide') and n.endswith('.xml')]
        report['pptx_slide_count']=len(parts)
        assert len(parts)==report['slide_count']
        for n in parts:ET.fromstring(z.read(n))
        report['pptx_fade_transitions']=sum(b'<p:fade' in z.read(n) for n in parts)
        assert report['pptx_fade_transitions']==len(parts)
    contact=Image.new('RGB',(1200,5*251),(223,228,229));draw=ImageDraw.Draw(contact)
    for i in range(15):
        im=Image.open(screenshots/f'{i+1:02}.png');im.thumbnail((384,216))
        x=8+(i%3)*400;y=8+(i//3)*251;contact.paste(im,(x,y));draw.text((x+5,y+222),f'{i+1:02d}',fill=(20,35,45))
    contact.save(ROOT/'Preview_Contact_Sheet.jpg',quality=90)
    (ROOT/'VALIDATION.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report,indent=2))
    assert not report['console_errors']
    assert not report['layout_overflows']

if __name__=='__main__':asyncio.run(main())
