import os
import base64
from bs4 import BeautifulSoup
import minify_html
import jsmin
import cssmin

# --- Configuratie ---
# De input-map met de bronbestanden van de website.
WEB_DESIGN_DIR = '.'
# De output-map voor het verwerkte bestand.
WEB_ROOT_DIR = '../web_root'

def get_mime_type(filepath):
    """Bepaalt het MIME-type van een bestand op basis van de extensie."""
    if filepath.lower().endswith('.png'):
        return 'image/png'
    if filepath.lower().endswith('.jpg') or filepath.lower().endswith('.jpeg'):
        return 'image/jpeg'
    if filepath.lower().endswith('.svg'):
        return 'image/svg+xml'
    if filepath.lower().endswith('.gif'):
        return 'image/gif'
    if filepath.lower().endswith('.ico'):
        return 'image/x-icon'
    return 'application/octet-stream'

def resolve_asset_path(base_html_path, asset_url):
    """Resolveert het pad naar een asset (relatief of absoluut t.o.v. root)."""
    if asset_url.startswith('/'):
        # Pad begint met /, dus relatief aan de WEB_DESIGN_DIR root
        return os.path.join(WEB_DESIGN_DIR, asset_url.lstrip('/'))
    else:
        # Pad is relatief aan het huidige HTML bestand
        return os.path.join(os.path.dirname(base_html_path), asset_url)

def process_website():
    """
    Verwerkt alle HTML-bestanden uit WEB_DESIGN_DIR, inline't de assets,
    en creëert geminified versies in WEB_ROOT_DIR.
    """
    # Zorg ervoor dat de output-map bestaat
    os.makedirs(WEB_ROOT_DIR, exist_ok=True)

    # Recursief zoeken naar HTML bestanden
    file_count = 0
    for root, dirs, files in os.walk(WEB_DESIGN_DIR):
        # Voorkom dat we de output-map zelf gaan verwerken als deze in de zoekmap staat
        if os.path.basename(os.path.normpath(WEB_ROOT_DIR)) in dirs:
            dirs.remove(os.path.basename(os.path.normpath(WEB_ROOT_DIR)))

        for file in files:
            if not file.endswith('.html'):
                continue
            
            file_count += 1
            source_path = os.path.join(root, file)
            # Zet het resultaat direct in de WEB_ROOT_DIR (platgeslagen structuur voor ESP8266)
            output_path = os.path.join(WEB_ROOT_DIR, file)

            print(f"Start verwerking van {file}...")

            try:
                with open(source_path, 'r', encoding='utf-8') as f:
                    soup = BeautifulSoup(f, 'html.parser')

                # --- Inline en minify CSS ---
                for link_tag in soup.find_all('link', {'rel': 'stylesheet'}):
                    href = link_tag.get('href')
                    if href:
                        css_path = resolve_asset_path(source_path, href)
                        if os.path.exists(css_path):
                            print(f"  - CSS inlinen: {href}")
                            with open(css_path, 'r', encoding='utf-8') as css_file:
                                minified_css = cssmin.cssmin(css_file.read())
                                style_tag = soup.new_tag('style')
                                style_tag.string = minified_css
                                link_tag.replace_with(style_tag)
                        else:
                            print(f"  - WAARSCHUWING: CSS-bestand niet gevonden: {css_path}")

                # --- Inline en minify JavaScript ---
                for script_tag in soup.find_all('script', {'src': True}):
                    src = script_tag.get('src')
                    if src:
                        js_path = resolve_asset_path(source_path, src)
                        if os.path.exists(js_path):
                            print(f"  - JavaScript inlinen: {src}")
                            with open(js_path, 'r', encoding='utf-8') as js_file:
                                minified_js = jsmin.jsmin(js_file.read())
                                del script_tag['src']
                                script_tag.string = minified_js
                        else:
                            print(f"  - WAARSCHUWING: JS-bestand niet gevonden: {js_path}")

                # --- Inline afbeeldingen als Base64 ---
                for img_tag in soup.find_all('img'):
                    img_src = img_tag.get('src')
                    if not img_src or img_src.startswith('data:'):
                        continue
            
                    img_path = resolve_asset_path(source_path, img_src)
                    if os.path.exists(img_path):
                        print(f"  - Afbeelding inlinen: {img_src}")
                        with open(img_path, 'rb') as img_file:
                            encoded_string = base64.b64encode(img_file.read()).decode('utf-8')
                            mime_type = get_mime_type(img_path)
                            img_tag['src'] = f"data:{mime_type};base64,{encoded_string}"
                    else:
                        print(f"  - WAARSCHUWING: Afbeelding niet gevonden: {img_path}")

                # --- Minify de uiteindelijke HTML ---
                final_html = str(soup)
                print(f"HTML minifying voor {file}...")
                minified_html = minify_html.minify(final_html, keep_comments=False, minify_js=True, minify_css=True)

                # --- Schrijf naar output-bestand ---
                with open(output_path, 'w', encoding='utf-8') as f:
                    f.write(minified_html)

                print(f"Succesvol {output_path} aangemaakt.\n")
            
            except Exception as e:
                print(f"FOUT bij verwerken van {source_path}: {e}\n")

    if file_count == 0:
        print(f"Geen HTML-bestanden gevonden in '{WEB_DESIGN_DIR}' of submappen.")
    else:
        print("Verwerking voltooid.")

if __name__ == '__main__':
    # Verander de werkdirectory naar de map van het script
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    process_website()