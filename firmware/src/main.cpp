// Start up: Select program (short selection, long enter), select timer (short selection, long enter), (short) start assignment
// Buzzer +: D8 -: GND
// Timer: VCC: 5V, GND: GND, SCL: D1, SDA: D2
// SWITCH: +: 3v3, -: D0
// 

#include <Arduino.h>
#include <String.h>
#include <math.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include <driver.h>
#include <timer.hpp>
#include <buzzer.hpp>
#include <button.hpp>
#include <wires.hpp>

#define STRING_LEN 128
#define NUMBER_LEN 32

String SSID = "HTB-" + String(system_get_chip_id());
String PASSWORD;

void handleRoot();
void handleAdmin();
void handleCode();
void handleNotFound();

Timer timer;
Buzzer buzzer;
Button button;
Wires wires;
IDriver *drivers[] = { (IDriver*) &timer,
                       (IDriver*) &buzzer,
                       (IDriver*) &button,
                       (IDriver*) &wires,
                     };

ESP8266WebServer server(80);

enum MAIN_STATES {
  SELECT_TIME,
  READY,
  GAME,
  WIN,
  LOSE,
  END,
};
MAIN_STATES stateMain = SELECT_TIME;
uint8_t totalTime = 30;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>HTB</title>
    </head>
    <body>
        <center>
            <div style="width:300px; border 1px solid #000;">
                <H1>Do it yourself bom!</H1>
                <img src="data:image/png;base64, iVBORw0KGgoAAAANSUhEUgAAASsAAACaCAYAAADxRW7QAAAAAXNSR0IArs4c6QAAAARnQU1BAACx
                jwv8YQUAAAAJcEhZcwAAEnQAABJ0Ad5mH3gAAP+lSURBVHhe7L0HYFVluv0993/nzlgp6af3k+Tk
                tPSEUBVUFMUOSO+CIFZABLuodGmCDQs2VASl9xZKegFC6C1AAiQhvbK+9ewQRSczZuY63uv92GFx
                ztn13Xs/7+9dz65/wrXuWnetu9b9L+wulZb9TNdgda271l3r/ld212B1rbvWXev+EN01WF3rrnXX
                uj9Edw1W17pr3bXuD9Fdg9W17lp3rftDdNdgda271l3r/hDdNVhd665117o/RHcNVte6a9217g/R
                XYPVte5ad637Q3TXYHWtu9Zd6/4Q3TVYXeuudde6P0R3DVbXumvdte4P0V2D1bXuWvdPdJcvX262
                /h1dU8tpSv8Xu2uwutZd6/6Jrikw/D39O7qmltOU/i9212B1rbvW/RNdU2D4e/qtu6aW8ff0f7H7
                /wWsmtqZ/139T3ZNlee/qz9S11T5fy/V19c3SzJuc7tfLuMfqallNSUZ9/9adw1W/6L+J7umyvPf
                1R+pa6r8v5eaAkNTknGb2/1yGf9ITS2rKcm4/9e6a7D6F/U/2TVVnv+u/khdU+X/vdQUGJqSjNvc
                7pfL+EdqallNScb9v9b9YWFVX1ePOu6QWlxGRX0NKuur+b1WhjTocp3yWcfhVfxWw31XL/tPJKPU
                yY6/LB+o4U8ZR4Zflkl/RTJeDZclS2AxUCv95LsiWWa9UpKG5coyuCwZJgHUuHzqsiy8cZlXFb1Z
                kvk0R01N25Q4rmyran6VoijTygrV1nH9GtZWRvuf7mTb17FcyvYSNezmhgGN69uwI35cr+bol/v4
                70n2dTV3ek19HfcnoSDLlXJIEHGZdXU13OdV7CU9CBj+SVEkTmtY8FpK4o6zUSarvPKpsEV6SlzW
                Mq6pGn6v4cRcVMP6cMTLNbJMZTSWhfuLv+ur+UMWJ5IdKDNVYouST1k2Z15N1cpCajgjJWgb4v+P
                0v1hYVVVSUAxaEq4F/Oqi3G2uhCl9aXcLUQEg+VybQX3VRV/1eMixy+RfSQ7RgJCCS5ChQFRyd8l
                /CnjsFdDUPyKZD5FnKqSMytnoJQxQARMlVx6Bb+VUyX8XsB+l9jKVTIwlOH8rszjCh3rKhjKEjcM
                rtrSK/2vlgSeSCa+WlJ+KWtzJOM2R1ynSi5UtkMZpZSzQn6Uo5TbtkRZM470PxzcUnkruA0uS9lE
                sj1kWzXCqnH7KhWWkn6/JtlOMm4zVE5dYqCU1krj2AAOZVnlFPdVdVUpt9UFFu0S593QcElclHLs
                osoKbs4K1HB62a0Sd2c4TLa3zEbZDxILjIuKCsZ21WWUcF2qZeTGZZSxAPwnq1jOsl+qYDMisSMz
                ETUGcyUHysZiPSH9GK9VOM+llsqSiziBzLiqHhUKJf8Y3R8WVrLDagmBSnFV3MtV3H219axONdyr
                1YxgBlN9HQNKWjNGgsRBFcctqy1HSe0lji+Vr5LAK+f+rGCsN7Q60vr8ulita4kiwpA/lYBVdjmD
                AlWMmAqqmi0vo4kNrVKPRLVKM9kQHPzG+SiTK4FYV17LDwnsnySgleVJqyiS8okaW8nmqJJLao5Y
                AJbjEsq5LaRc0kv5UlPNgC5hPZAtLCvCdeXHL9OT/66u7n457Or0powFKOBWKGFp5E+2Ui33ZSVr
                cjk/Za9Kf9nj4m6qm6Vaqunt90vVSnlquSXYGMqOk2mVRlACjNurvrySHLiAsvqihqC4smqN63FZ
                GizZ6QzKOlK35MJFNrKVLLeUVvyrxLM0dlVkTjWxV6OATtZU2SmXG0os0ClUxpE1l/iXZppw4zjn
                +C1f4lzqgrgo2W7lJSiuZAPLDETcsoBMHJ5sqT9K98eFlVSYWlbdGm5uBoLs/Hru/PpyAkSih/uj
                upphK82SQIQ7GTVsUYrOoPRINvJTtqE4dQdqMpKA7AwgNQUVWako2fvrKkvfherE9ahIT8GF7H04
                tTcXR/cewOHMTBzNSENeVjYu5BzF2cxjOLz/CA7sP4jcrP3Yn5qF/Tk5yD52EBknc5FxZD+OHziA
                85n7UZSWhYIDqT/T+ZxUzicVF6+ocH+DLu1ruly/lIx34UASzjdD2JsCZKTgFNcpd99+HM05hBMs
                9+H9WUg/mIr0Iyz7wQM4svcwcvbnYP9+rs9vqBxul0Zd3X/fvn0/U87+TBzan4zT+5JwcW8S13EP
                inN24eLBXSg4tBvnc/dwGyWhJDuF2yC5mZJxm96Gv1RhVjr3VQqKUvfgbPZupO/fg905Wcg4cASn
                ck7jWOYBZGWlIG1fMnL25eBA5j4cTM/G4Qxq314czOW2zc3BGa5bcfpeVO3KZFnTcSknDeWcf2Vm
                Ckozk3ExOwnnOI+zOclKDJTsS0Nldiqq03dz+G4UyfrnpqLoUAaKD2bg4uEsnM87gIvlpwiwQv6J
                txOICRypCsZ/GZvASsUfc0gDqKRN+qN0f1xYKTuBEJJWQlo1OpP6UrY/ktfxHxs/VFXRVYnLqqEl
                LziB6pSduLjkc5x4600cfuZZ5D0zFheeex6Fz7+AM8+MQ/7Y51EwbtKv6ryMx/GzJ72MVS+9jg9e
                fB0zJr6MV8c+i5fHPYPXJk7AVPafOuENTJr4Cp6f8AJeeuZ5vPjEODw38Xk8/fpEPP3Gc3jhxWfx
                wfMTsGr8C0ga9wKOTZrUoImTcPyKTjw/CSev6NSEBp0b33S5fqn88RNxauL4ZunMpInYy7J9+uxT
                mDxuLF5+/nm8xN8Tn38WT734FJ59eRwmsWyvjn0JE56bgOeee+431YQJE36mpsYRfchhaROeR874
                CTg57jmcHf8czj0/HnkvjMfJF5/DqUkTkMdxzo2byG3A/TS+GZLxmth+TSlz0otIfn4ido0fi+8n
                PonXJo3GmBef5jZ6EW++MBVTnnsdk1i+Z7lNx096AZOenYBXnhyP154eh1eeG49JL/H7K89hwcsv
                YCXjJvuFGTgx6XXkP/8SLo5/EZeemYTCZycq5T81YSJOMAbyJr6AAsZoIfd70TPP4fyz41HAYRde
                fBmFL7+Gc5NewvEXX8LRmVNx5puPcWnPGlSdZbwzu2DOgJI6qQOsFFI3WE/k2Cn70JvJMSv2+4N0
                f1hYNRhjcUxsJwim2pJqyD6RLKua27+knK5KHFYV7frZgyhaswzpT4zBxsgEbGltxCH/YBz1s2Fv
                Sz0SbwhgvyDk3mjEqestv6oTrYKRY/RikT4Uj2osaB+ghaWlL27+8//Dn/7yJ/zpOur6/8Kf/nw9
                /vTX6/Cn//orWvzpOrT601/w/2QcDv/P6/8Ey5//hAf/6zqM/0trLLpehdwWlgbdbMHBKzpEHb7J
                giNXdPRGC07eYMHp662/qlM3mHHsZl2ztM1Xixmt/XDnn/8TQX/6E/76//7EsnI9/uvK+lz3H/jr
                n/+Cm7kef/7PP+M///M/f1P9+c9//pmaGkfU86+tsPMmEzJuMOHQdUacvN6A0zcacLyFAUdbUjcb
                cYzDTlxn5vqbcPoGYzNkanI/N6WVKhu+0JjxQYAaz/nehNjW/4GbW3D73PgXtPyLL/z+7IPr/3oz
                f9+AP918E67/r+vR+j/+Cp//9xfceN2f8R83/QnXcfzg1jegB+Nmst7Dbe/gPrCzHHYUXGdD/vU2
                rpMVxxgLR1rzszV/t7LiDH+fuZFl5frntbDiXKsQFLQMxvG/GrHvr3pkBARjf1Q7nH64F6o+/go4
                cpJ8qsDJ+ktMS1lJxEpJHSG4xF9VXya4JDP5g3R/WFiVXjk+UcsNXlNZgzpyi0YXJeRTgRw3KCWk
                KtnzQC4KZs/E1u73Yr0lDBm+ZpQGOIBAFxAUBuicqLF5cCnEjfPGUJzTBv+q8nWhOKV3YY6PCg+0
                8oHb1xdm/0AE+RFYGh9cZ/LHDWYNfLV6XK/XwlenRwih5lSZoVYFQcNxYvX+eMZoxtdGJ1KN4ThK
                +J0yBCs6TeXpg3Hmis7qflK+LF/Kwe+/pnydHfl6Y7N0OsSDzaFejFGr4PVvDVWAL3yD/OAT2Bot
                1S3hp/WHVqWGxU/LdVBDpVL9plKr1T/ql8OCgoJ+1DCNHkl6G/ZpCGOVFeepQm7b83oLzlJndDac
                1dhxLojSskLrmiPb3+zjv6cCayQu2qNxkjGzwWLBOGMAOmhaQufXCv4tWUYfHXwIoRu4Hn/RqdA6
                MBBavyAYAjhMG4DrdDfjRtV1sPjfiG5+gRjHcq43hOIIYy9P58B5TSguakNxnv3OmkNw2haCM9YQ
                5Fu4P8125JlsOGlkY2VgufV2jmvHhUAr8vzNOMj4SjdbkeR2IPv2h3BuygLU5h3DWZTh1OUSlFTL
                MSxWEgKrurwatVX8ceUY6h+h+wPDSs66VaOKsCpnPi4Ol8ke8/XLKJFWpKIC1fsO4Mz0edgb0R4Z
                Vi8D3I5TahvKTWG4bHOhTG9FgVqH01od8kNCcNEajELzryvfFoZ9YeEMND/EBraEVhMAo8EEM+Gk
                NhFSZjV8GKg6HzXhpYGPXg87W2MHpdEEIUwfiD5WE5aGRuGIORKFOheKCJerl1FEFZsadMnYoJIr
                kn6FJnszZON05mbpYrAHiayATwQGwePvA7VGhUC9Diq9GgEEq0rPCsd1sQRqub4arsdvK62W871K
                Vw+7GmQj9AakWWw4aCBgCJpLBFK5zoJSoxVFJisuGm24yEpcyEpcyH6FrLy/Lhv18+3/91StceAy
                G6oqcxiOWm34mvvxaZMOHbUa6LhtgnRmBHG5fgRZC5sRfjrCneXWyPY0BKGF2Q+t9K2gZ9zEtmqN
                +24OxDd6M3IZe6dMjEFDCPdHKIotoSi0h+BCaDAuhthRbLehyEp3ZTbjrM2KfDtBZbGjxGBHJeO6
                Us1x+Hma2+QAYzFH58WBLj1w+J15KC45gQsoZv0gpeSQiRzGLavF5co65ZDJH6X7w8JKTrPLmZja
                y3XKsSk5nZxPeyVnT+S8DfLP4exHnyG9Wy8cuF6HM0FsrYxhOM/W6qyZLbCVaQID7ZjZgMMmPY7w
                dz6D76KVQfArOhUShu0REein94HJ0AotLRoY2PKFsNJYzCZoGbwqVSAsN/jRnajpsHQwBephURmg
                ItiiDSqMswVjnyMBFXRVFWoLLmi0KGTwNaroiorZml66ohICSFTE7xdYYX9NF1kJi7iuzdF5gmq7
                yY3HfAPh8PeDimVWmSzQGVhmgkqtC4SOoDVyfTTqn4Pmt1BzYTWM5dnNiruP6ybuopANTonBQoAT
                VARPAfsXEFjnCawL/H3B8uu6KNuqif3clKqCrKilq6vkcs4THFmEzCLuj/56E4wsn4/RQCiZoGYD
                EMj4CtTpEEh36B8UAF+1PxuuQMI/EHp1AEJ8fOG9sTXepfNOZ+wd5DzPEj4X2RgVMZ4K7QIqgjTY
                jGKbif1MOGsyoiDYjvMOgowNrDRIpWorKugwywmsIkL6DIF1lilhjoH79I5uuLh2OaoKTxFVlaio
                Ja3kqHrDQatrsPo9OuV4lUg2Nj8K6a7OVVegWE7BVxQB23cgY/Bj2GmJQKHKwdbKhUt0U/kMiCMa
                NfYF+uJUsAWFEV4qAocZXHkcls/g+zUdDnZglcuNe3StEGDywfU2PXR0PI5AM4INZhgtBmjprIJv
                8mMF1/M7XZe/Dga1ni1vANoa1ZhMa3/OHo86vQPFdC/HzP5sMZnqibiMc1ckyytg4J6/SvmE2Llm
                6Ow/oTPBkdhiicSjfhrYmbIEsMwqglEvn6oABGn86Lb8oVMHEiDiFH4Om/+u/h6oRFfDaqDJgI10
                GntsrNysqHkEUz5BdY7QOUXncYKV/gT34wk6rzOElfT/dTVvv4uKCMiL0rCFGHGOqf55gmWLwYFx
                TEVNKqb/hLoP01VdgAZGQkhcaCBTQV8/Nly+vsp3AxsCs84AE1291i8Abxn12Gq1II3lPUJYnTHa
                UcB1K+C6FNBFnbcaCWIDXZeeKbuBsGJZnGE463TgNMfJU5tQEMhxgiy4SId+3uRAhdWFU4EmbPXR
                4tDgEajfvg0oK2L9EGSx2ki9oeSQ7x+l+8PCCnVsFuTgoOTgVDWBJdfgVNdXAHlHcWLKNCTG34ad
                mmAcd3pwzuVBYWgYSlkxK+iAqpiSVbKFLNcytSIwLplcbKHD+D30V3WKwbnKEIzufi0QqGmN65gC
                BPow+K5jqqSiI7EbEGDTwuAfCCtTAjuDzxZogI4B6m8IQBtDIF7j99MaN8pURpy2+SPHHUgnEKro
                ouknFUpKIDI06JJIF4Ji7a+riOMVcPzm6ExwLDbaYjCQsNK19sdN/Gzhp4NvCz/cfMN1uKnFX9HS
                9wb4+beEn58v5febyt/f/0f9cphU8kb1I8w2B4cgxRKCw6zYcrzpInWB388SrqcJj1NMpfK47oXs
                X0yX8Wu6RBfW1H5uSmcIj9wwPbLCAnGA6dwlxlGm2oE3NTYEE1QtrAQsyxjqr0Gwvwo2AZNeT0BR
                QVrYAnQIVbNR09ugJ/hacfgbBiN2sJHLZkp7SsX1oUO6xJSuVEfROZbpjEzz9KimO6/0NaCETlzc
                13m7AxftEtMuVNIVVxvcqDQwjvVOZhEmnKMDPtlKh+0+BPdLU1C+fz8KaafyCKzCqhrlkO4fyFj9
                vrBqvDCuTrnl4G/vXxKvJNQXdyoXcl6u5XDpKbb1ylaVD+mljKlchMmvJRz3ElBeX4easnxgy1pk
                t+mCdD8rDgYwx7c62Co6aPml8gez0jMYGKDFcpyIuqhnYBs5nEF+kQFSSJWwApdYHLTerMysBGfo
                MC4ycCoJkIN0aK/aXbiFLala3RI36Xzgp2GL6RMAa6Cm4dgVW8tWGhX82PrqjVrYg3TQsVUMUvuh
                G53UxzYuzxwOOTh8xqjDSZtROcAvunC1WA7Rxat0nmW4RNXxe52RULJwHmYjznD+JYEWVAaEMi3w
                oEjvxVk6jzMiVoyrlSfSW37UQVs4lls8GOzjB9PN1+EGv5a4jqlsK7qsVje2QKsbboBPyxsR4NcK
                rVu3QqtWv6VaUz4Navm3anmVBgSosYtlT+f+O8iU5xQlJxwusHKflwPqsj1VJuRRkvZfZENURABc
                CjCgOEDL71ruYwO3rVk5rlds4vbjNixkTDRHZ5iCnaYjz6PTySOEzjBWsswezOP+CPe9Ga1NQQgi
                fMx0O0aVhpDS0p3qoCeUdEz1DXRTJrptI8sgjVcg+w8I8MHneh1yQ11M45jSas0sN1Nzlu+SxUlg
                2QksM+HFVJBQlPi8wMZS4kAat0I6qSKuazFVxIZUYqTQGs5xXDgTYCJUTcjpeAvOvD+f1aUIZXUV
                qKyXi2fl0mO5LJ4NvNwScFnqFFXLSlVzxQzIpUFS/xrrYKP4u77mMj1DjXJ5UF1NFesrp5GLTlmD
                L3O+cjOcHL+Xy4gqK7k8Zj0Ny2MP9ldumbpSr5vT/e6wElDVMm8W/RJasj0q+b2aa1gn14XIpQeN
                KyQrxz+5ilquHSmnj5LNrUxUytGKCSu6reLj2Tj7xivIVgXjWAs9ChioBYRQc5TPYMxnSytBX8zf
                lwQotlAGphXHGHDnuNMr2KJnhLgxONiJNpog2vyWygFTf3MAAzGQKZS6AUwMxBv1GtxsCSC42J+V
                RGClDfRBT70Wy0O9KLZH4jyXd46BeI5AaapMTekcASoQrSfM6syspHYTjjMVPW7QoZBwvuQfinx1
                OI6YopDD8UT7zSE/0z62zFdrp92Lj5g69KdrsrT4L9zgfyP+ovZHiyAVWl3fCi3/cj2BdR18Wt+E
                li1aoMVvrJtvbtUsPcLGYL01FFvY+Oy2hiHd6sQ+ljuXlTqXFTeH67KXbjbLaEG22YUDBieO0V2f
                CZJUSYcCtZb7mPuSzkOOb13gtrhAYMlxruboTIik5Q2O7QIhcoINXaY9HB+wLHEtboCvMQABBhN0
                hM7Vqe0/0l3cpnPYgB3whBNIhJWcoWX5zjDlL5Iz1YTPJQK6kOvUVDw0pSJrDMEVgXN09EdNWqQR
                iocffwxVpw6w3sm1+PJXw+pDgFRUkFOEVJWcJpQ0haphxRKJa5A6dkVXbrltuD9R7mHkODVMa+Sz
                Tq7Kr5M7DeRK/BKOJvW7EVZ1zH5KUXu5iP1LG+76+CPAqobEblQjtGTYZYEXN5RyVbpyIPDKVuEW
                khWXTVBNVXAjF3KFL/CXnAG8cvKPG6QUpzavxJrbO+OIji2MhpWaLU0+W87m6BxhdU7OEDIQC9ly
                FbFSXJBTx4TVccLnjN6MYsJqFyHWw2RDFJ2HTkNXZfRFkFUFo1ZFUGlgYyoosGpBx9TSEgg10z4b
                YaXVsUUlrPoRVitCmZbaI5DP5Z0lrM4yVWyqTE3pnJw14mc5y3jJRCdhM+EYndkxCfAgK44EhSJZ
                78YaVqLlwVRIOJYFe3+m7+yen+mzkAi8Qcf4gFoNM1O9Fipf3KRRw4epi99NvvC54Wb4tLgRvhzm
                S6BdnZr998VUj47Opxm6mxV5AVP699xefOIOx5fuCHzjicQyfn7Hft+63Pja5cSXTge+pDP+nu5j
                K5VJHeE2PmmQ0/4mykh3KdudjQUhlK9s/19XHlNQOWZYKI0a40VglUVX+pElDO1b3kRQCayMhJW5
                STA1pdt9bsY0NmyZLi/dsAWFbNQEpnmEVaFVHDKdH+PkAsvaVDw0pYtGN4o1rAP+dGyM0zSm9bl3
                90Txd6tonFjfWK0EJHI3ziU29KXMTipoeuQaalZD1kdK+ZMbvAg3Oq+qy3KHqJxTLGMfudVI/qRO
                Kl4Jcj91FecpJxlluLBIbpSWG7Eb7vCpoREpRk39RfYvVR5GcMWnNKv73WElTkoA9UtgKSJq5U/u
                kZIzfXLGT66lqqA1rVHu9uWWFEwrF1WVEU7lKKnguOIsRSdO4Mi8WfhCpcYptqYX1HL9DGHA1K9Z
                MthwVuAhwGILfZGwOk9XcpZgOkELLoF+lgG0hm6mC1t4V6AfLb4fgix0WDYtTAw4G62/RWDFgPMx
                6eFjDoJaH6T0U2s5nJZ/GC3/uhCPcgbujN6GPLamZ9lKN1mmpiRQVa4NMuO00YijVhOO2MxsQW3I
                1YRgsz5USTNfcoZhANWvUWFh6Bvm+EkOB/o4QhUNp1McRLdyCyuJjSmOlumOMcyFsLBwRNAFRjk9
                iIr0IDLeC4/XA4/nt5WboGmObnE60Sc0GP1DQzCIGsayj+B6PBYWSoVgRFgwhofZMTTMhjFWK15k
                YzMvxIVlznDsdHiw1xaGw0yvjrHhOaE1cb8acUqBFvd9M3TaHqwckL/AGBEonNDRpVrC8bnZia4+
                rdgwBSCQ6aFOa2wSTE2pI1Prl9mA7XS4CUCZt5H7mEA1ywkVNy7oGIeMD3F2TcZDU1IxO2DDVc2U
                Mc/GxpYp4qmou3D+qakou1jSYKBIGWZnStVhWy8HVii5V1QOwguYLhBLecTTURTjMBPIHJqEfey7
                H+eRiwIc4edJ/s7n8BLah2qOLwmgwEtqMqsrYSRJklzS1WDSJPUs4ud5lFedZTmYFjWz+11hJV0j
                sK6W4qqoWoKoUiBEOAmo5AbNMn4q11TVEluVFagr52aVCz659tW0npe4RSqEzkT6pQ2bkD1iJLao
                9DgVaMS5IO502vE8uciyGTrDAMwTCbSYHuQzrShgi3yWtvokW9Ij7H+IsPqc6WBEKx/Y/X2g1wcq
                oNLbDTAxDbMwzZCzPDoGbKDJCH+TGlpafAv7ydksJ6d5gq5ra4gX+XLGRiqAAsimy9SU5DjJKbUF
                R9QG5HI5uRYTDtrsyCFsdhgdTElCMJaV9qHwUHR2u3Bro1wu3ELX8aNY8TsRYqI7wty4nZU5jlAK
                JTzs3kg4vNGI8saiDZUQHYP4hGhEt49CeEQ4wsN/W3m9zVMU16FNqAMJVHtHGDoSwFJ+WZ9OrjB0
                cNPhXFEXuqsH2G+E243JTLE+c0ZgU7AHmeYwHCLwjxAoR7RMnwmHPHFNzdApbtszhJ0cLshjg3ac
                rueAORxLTW485B8Agz4AQQSPVkvgNwGmphTHBuxpnQZr2GDImcBzUh7C6hRhlc/5ntMyBrnPz7DM
                TcVDk1LT+RFWJVqW0WrHUTZgx1ThON6+B47t34PzJWdRXF2MC1X5OFmTjWOVaThYuhN7L25E6tkf
                sOvkV9h65CNszF2AtftnY82+mVhzYAbWHJyBlTnT8cPe6fg+exZW7puPdbkfYfuJb5CSvxrZxVtw
                qDwZF2pPs+7KoRpWTaqcsBJoyWOIGm7Tv3QFVvKYiOZ1vwusGmF09fer+/3YiVeU53/QbdUz9ZPj
                U/J4DTksV0GoCZzqxGNWiK9sgJQ8DUNSQZRV4MS7H2D3bXcim1A5HqhjQFm4sxvODjVHeQzAUwTH
                SQbMSaZ7eYTVOSPFHX2KLegBtlhpdFXzGODmm26G0d8XBoMaBpuBMsJoZJrH4DNoGIQ6wslshMqo
                ZUqohZmwUgUFIZKwmsDWfDdhddYahuN0QycZhGcY9E2VqSmd1jL41GbspVvL5HL2my3YbwtFip0p
                kM2JiXQc93uCERcdijZR4VREgyLDEX+V4iK8Pyk8AjGRMYiIiYcrrg1c0fHwRMQiyhONOCo2MhKR
                seHw0FlFRvG7/P4fUDjL6WH652Ha52X65yVUPeFURBQ8kVFwR0bDHRUNT1QMPNFcpygvukR40Nfj
                xsthku56sd3qRo7JgcNyYoGp+RHFXdmapROMi9OMD7lS/iQbtOOMjYPGcKw0eDBIDgHoAqDSqaFh
                jPwSSn9PniB/DOHnErr4Y4y502wMTxFWJxm7Z40u5Gkk/oIZJ82P5dOM19Nqpql0WPsZs4fZ73gr
                M45YIpG6ci6yD69C+qnN2Hnse2w6NhfrD8/CqgPTsDxrMr5OfQlf7H4eixPH45PtY/HxtmcVLdo2
                Fh9SH2x9Fu9veZafY7Fo+3h8umsivkh+Cd9kvI7vst/C6gNzkXp6DQ4XpeFc1UlculymPM5G0kXW
                XNZmcoDfqmvLmHIKyprX/S6wutpB/cNO1kKS30ZxPWq5ltUl9ai6SNd1rhLlJ0tReqQYZYcv4cLx
                EpzOK0XeuUuo3X8Uuc++gJ1hkTjMnXOIQXiOKcB5u4s7mc6oGTrFlPEEYXWU0x9hC3pSXJXegQJt
                GHd+GLIJrq1mG15jOud/443QBrIlNepgtBphsFCElZz50UkQElBakwEatrJyJsioodMK9EcbPx+8
                zsqREezFaQb+Yc7vOCF4RudoskxN6QRby4N0VukEYCrnu4+VJ9PmwsbQcMx0uNDXFYr4iBCExoYh
                uk3kzxQVH/GT4iIQGReuyBMbDVd8PLxt2yOciohNQEQE4eUmJAiGcC/TsHAXnJFOREVHITqaLut/
                QFGEUGREHMIjWVbKHRUPZ3QbOGMS4IptC3dcO3jjOsAb3xGOtgkIjo9mmT1IcDmYOoZhGp3VSrsX
                WVYPDpnYANHFHGLKfpIgao6OcxppXE4xfT8mcDGE4Yg+HBv0XjyhMSFMG8B0X0X9HEj/SHZ1EB7i
                fnyPcZArDRhBdYKp6QlpaPVhOKEOxVFtCJ3938bC31OeyUkIhyAtyIxMlvMo09+8ACPLa8Lmaf2w
                btOr+GbXFHy49QUs3v04Fu8aQz2Fz3aNJagm4Ms9L+Lr5NexNHUqlqXPwvcZc7E0ZR6+TpqPb1Pm
                47v0d7A8Yx6WZb6NpRlT8VXKK5z+eXy0nWDbPg7fJs3A+n2LkJq3CifLs+mxlOvnlWqtHMdiXf81
                HPyy+18FK6GurJDkuXVlnO5cHepzy1C9qwCFyw7g6IIdSH/5e2x7cjH2jViMA2O+wt7nvkbaS1/g
                4kuLcKpzfxwIDMUhpkc5IVYcczhwik7jBHdec3SSsDrG1vawxYZcgu6oIRRnCKrzGrZuOjfSTGFY
                YbZirFaNm1u2gEpDV8VUT6TVE1IGtqb81MgtFgxCvcGggEuvkVPXegQE+KGTry9mas3ICQ5nMNoZ
                nDYcNTK14HKaKlNTknLlsGVP5XpmqEzYz9/JNi+WhEXgSUl/wh1wRbpgj/EipI3zZwqOD/uZ7HEO
                RcEEliM+Dp6EBES2SUAM3VVsTBxiqNjYWMTFUfGxiIknwCIIryZSuf+evJTnVxVFZxVLiEbR9QlM
                PZSL390ElyeKgI1uR6h1YJk7Ibx9RwS3iYeRsLI57egSEoyxwS58HhyBZHskcpkO5hAKB+lkTtC1
                NEfHua+OMyU7RlgdZtotsDqu82IH9YLeDq/AShMIFRu0psDUlHR04rdrDJiutWEvXd8Rg4XzNeOY
                pJlaB46q6N6YCh5kY9lUPDSlsxYXDtI97iBgDzmike+KUU4WHbNrsbaPFysWD8XnmyZg0dYXsSZ7
                JjbmvIPtTPuST3+LvRfW41DJbpyozMaZ2iPIrz+N85fP4VzNRZxl6phfy/QRRcqxqnP1xzneXuy/
                uB1JJ3/A1tzPsSpzIT7d/jIWbXoeS3ZP5nwXc7z9BFZJw8F4gkqujFDOLP4TwPpdYNVUyie/5Sxg
                dXU1KivpmMrLUVFSi4oC4Ex6PrK/TMaOycuwYeQHWN9rHjbeNwcb7noba29nznzLVGwNfxm7Il9F
                YsJr2NTpeWS3GYFTxttwphV3qkqPbFcwDgU7cMIoASUt4K/ruMbC3N5CgNixjzv2EHd6njoMF1Qe
                Op8I7GFr9TVhNYo2/wamcyrCyGAyN1w/pWJrKpAyMP2jm5J76/QMWINa0kKBlgF+/q1xu48v3tES
                iPZwttK06Da6OAZVntbZZJma0iFCcx8rTQaht582/6DehZ32CCx0haNHON1UhBPOCDoiOpA4VvAG
                uRHndSNWkQuxngbFXFEE00VvbBTCmeJFRIQj4go8PEwRXUyjPAILuqtIb4QCq99cTEubo+hIpqyE
                TwzLFU55WDYXP92SIjIFjI6KRXx0AtoSWu1i2zEVjIU5wg0D0+IEVwhGOtx4LyQC220R2EtY7ScY
                cvUEA91Sc3SU6fpRpmRHCKZcWwOsTmq8SNKGYzL3Y6QCK38Ecf83BaamFMgsoC0bnldUdMt2Dw4a
                6O4FVnRax9RMV4PYOBFaOYbmx7JcknGEsbXVZmN8RbKx9dCdWZFuUGOH24B9n76AwydX41hZOs6W
                5qCg/BAuVh1HcU0eSurOo6y+GBVM3youV6G8vhrldTXK0Re5B7eKVVmgI8ZCOQl2uRyldcUoqirA
                hfI8nCjOwZYT3zKdnM40ciK+3vMKUvKWIL/mAMcvV9yVXMQtB/nlzGNzu98FVr/sxGXJWcCKigqU
                lJTg4sWLKCgowNldh3F89g5kjF+KLT3nYfUtk7Em/lVs7TgDyfctQsaAb5H+2EqkPLEW6U+uxr6n
                V2P/U8uR+tgH2NXhUVbizjgTKC2KBSkOuiRJ4zQOBlZosySwOkJYHSCssu0hyCWsTqsEVl6c1UVi
                p8mFxQTMUL0G16sCoTabCSsLtHRO/v4BCKTjUhFWKsJKRWuvV6thVAms6Lx0Bvj6tcKdPj74gLA6
                QVgd47R77XYcFlhpaNubKFNTkmuK9lnCkEV3dVglFceDbXQKUz3h6BLpgIOuyh0ei3Yeugw5KK2I
                oPE0ysPUzsMU7ydFEgKRhEA40yVPqB1uhx1hBH6INxQ2dyhCnA72I9QcBBfHb+qM3r8uN7yEqJdp
                5q8pgq4xMpwOxsvyUSEeG4I98hkKl9eJCK8HsXRfCeEx6OImrL1RCCasjFEOAs6JQSz77NAIbLCG
                I4MN2T6m/bnc53KbS7MkoGIafkgfjBybgCEMp9UepuMRmG52IkYTAI3GD0HaoCbB1JR8GRvRKgMm
                BJqRQsd9gLA6QlgdJayOiKsKcmC/hg2UknI2HRO/VLGRaaTNiK2hTF/V4cj39+Iw3Xuy2YbsFlqc
                fWcGSs7kEDyVqGN9vFoNT9et5fe6n/VX/MYvJP1+FEHG0Qi1KhyuzcH2M99haeYULE4chx/S38Te
                c2twsfo4YSXn+utRXlWGGrmQtJnd7wIreQC+shZM9OSqjEpuoIqaKlRcqsCl40U4tiMXB9ZkIPXV
                77Ctw2RsbfsadnR4A8l3zkZOv8XIe249Cudl4dLXJ3Fp/UUU7ijBuawaFOTU4GJmMc5v2IvkXs8g
                09UZh4wexa3stprYksgpXAeDkeBphg7RqUgru89MWDEQ99H2H1PTUqucbJlo9ZlqzSdgemhVaEEw
                acwW6I0maOieAgRWTAsVUImzUtNZEVaGID1tvomuSw9/n5vxkJ8/FhMyx1hZjjAo99qCccBMq68L
                +5vy/D0poLJSrDz7tHa6Axc20Cm8TmfVLiIMDrqNcMKqrbctXHRIYay8Droldxwrcdc4dLw3Hh3v
                b0O1ReeHb0GnB9uj60PtcfcD7dC1Wyy63BGOW7qGo2O3SLTvHo2Eu2PQ/q44dL6zDbre2Q633dMW
                Xbo3qPM9CQ26OwG33t3mZ+p8Dz/viUenu2Nxyz1xHL8NutzbBrd0j8ct93EYv3fm8C53xaIry3IH
                dfv9CbjjgTa4/d443NGduofl6RbD3/G47QEOuy+W/by45X4vy+Zh2ega7/Yi9l4Pf3vRsYsHbePd
                iHJ7caszEgnuCMV52WO9iIp24xGCcUqoG6ssbiRzW6YznctWgEWn1BwxJg4wDcyh9jMNPEgwHNW4
                kKYLx0KzG50CCSuVPwGkbhJMTcmPseFio/ZooB6bQsKxl8s5zDIdYhq4nzG8Xx2KbIImW9/8GJHH
                /hyzGLAr2I7jgW6cDYxgoxiOFJsTe/9Tj4LXZ6D8wCElHZPjL3KBptwQIo/hrq0msGoIq9paAog9
                lXyNn8qIrMeKrqKTDG4cRUZnL7mQ4XjNPuw8/Q2Wpr6OT7Y9ja0HP8DJ0jQSQB5KTY9VnY8a5QHy
                zev+7bCSdaou5BaQs3xcgTLmufInl5XJGxVq15Zgz4DvsTJhAb72vIolbZ7H9oHv4ui87bi07RSq
                T1WiqqweZVW0orVyBUgVtwtdWVUdCpUToJzvhRM4ev8AJNsisdtoRxqhk8J0LYs7PFvTcGtGc5TK
                dC6N6Vomp8lmK5nKFCGLvw8yjTuktTH1jMNzTPvifFs3GXRNSRNoRJDahACDBrqW/4XReh2+47z3
                6d3I1YQy4F20/mzxLMFNlqkppdDxpZpY0biOa206bGZr+b3ZhTcJwM5OVmCmalFM51zR4pqC4Q4L
                h5PuKuGuKIyZfjdeWfIAXln2MF79YSDm7BiHKeuHY8a6kZiz9gnMWjUa074fgbe+G4bJ3wzG618P
                UiTf31w6FFOWDVfGEc1cOQozVjymjD91+aPKMJlOxlPG/X4IpqwYgJeW9sTr3/fFzNVD8PbaYXhj
                9UC8tmkI3twwDFO/H4gZnz2Cd9c/ifkbnsCc9aMwb+NjePuHQXhn2SDMXdIX0z7rhbeXD8OsjWMw
                54fHMOfzgXhzzQC88cMAvPbtIEz6ZgCeWt0Tk37ojqdn3IIHe8fAxvUNj6FjjPIyzWRqy3QwnCly
                d6cVL4ZY8AMbiW06C7YwXnbQwTS1nZtSBhvCNMZFOlO2/UYXkjU2pDIlTyL8lhvd6HVjIDQBavzV
                0PxLFwxs5KwqP3Rk+rggxMN5huGQLpQNWQgSbRbu62CkEYpp7JdBWDZH6VyvLH7u1zmQGmhVLorN
                tgYjVWfEGj8Njj07AWUpyWQLPQ6Bc7WaOmwjncKnK/rpWJN8IZ0aiadcli6JXjkq68pwsvgwdhxZ
                ikXbnsSGo3NxtHIPh5AD1aWouXyaUxdz3OZ1/3ZYKZ3QW0hNRMkVsKUsbtm5Yhz/Ig2be76PdbfO
                xvdd5mDnSOa1i1JRvuY4sL8cOM/pKhsOyMmjLSrpxurkwtD6GtRWXuac5O02HO/cMeTc0QNJJg/2
                MPBSmXalMaiyjGw1JU9vIuiaUqrBiHTuzCyCqQFW3MlaI6FCsSXd4InBs3RS0X7Nh5U6yIAgTh9k
                UMPU6q94ikAUsOxXYBVCWDkbYGX+7WHlifEiPtqF+NgExCTQrfTsgKlLHsWcDYMwZ9tQzNn+OBbs
                eQ6zE0dj/o7ReDfxCUULd4yhHle0QMbZPvpHvbNtNOZteQzzt45Sfsu47101nfSTYfO2jMSHHPej
                xDGYtvsxvLFnJJfzGD7aMhpL1o/GNxufwNcrR+LLxf3w+az7sGrmI/h+Tm98tagfPv5+KN7b/QQW
                ZD2L+elP450to/DJkiFY+v4AbJjVD5un98XiD3pg8bdD8MXGZ/HJnkmYmjIK83cOwvwv++K5sQ8g
                Lq4NouVM528Mq/RGWKmtDbCis22E1Q+EVZ+bA6EnrK6Xky1NxENTElhZCKt2aj/MDnZhF11ULuEk
                sNpJWKWZuMx/AVaZXK99nEZglUVY7bOGIENnwsYAPXIfexLF27aj7nJN82FFyb1+yqvllBSxmmKd
                rK+km5J6Scm7DijpLxeGFlTlIenUSry7YQyWpjEVvLCN9b+C5qOGrop247JcQtq87veBFWFTy9y0
                sl5eUlWF2opqHFuaih1DF2F17CvYcucspD23DKe/24u6I2VAPikt91Vywwjn5Orairpa1IhnlaNy
                yvVYQBk/L9VeAk7kIj2hm7KTkxk4KSojMkLYkhAAclwng+lSc5RmMBFWJsKKaSArf6rBhkz+zpED
                2bTWK5xRGEWghfv7NBl0TUkTZISKsFLr1QhtfQOe4zJWCax0LhzQMJiYzmXY2PqZGUhNlKkppXAd
                mwMrN2EVFxWKhHjCqm0c7urTER9vmYAPdo7A+8kj8W7yk5iTSPH3nB1DMHfHIMxLHIIFu4fh3aRH
                8R7HeT9lJD9HYOGeR9l/OIEwBNM39sbMzX0xe9tAzN819Mq4I5RPmVbmMWf7IHy4dRgWE1pzEofj
                TfabtmsYFiY9jiVp47F8y5NY80FfbH66M/bc7caBOFbEW53Y2C8an7/eGe+uG4QZGaMwM+UxfPhd
                f6wY2xlpd0fh8C0RyLkrBiv7urFyQmesWzQUK7hOszje7FRCeN1QPPXWIzDEeuCNa6az4rZsajs3
                pXSrxIkVGWob9hFWKQRYGqdPtriw0uTCoJYqmAmrG+mgm4qHpiQPNTSpAhCn9sebjIVtdENyMD2b
                sNplJRhZxnTGnwCrKTA1pTQ21FLebC1jhbDaa+Y8LSHI1pmxI5Ax3ncozq9aS/L8M7CS10zIKygq
                CRk51lXBqlhOVaCuToAloBKnVoPS8gpU8vvFurPIOLcG7214Ekt2v46Ms1sIq0rlDWFyEbiSZjaz
                +11gdZmgKefKlMprhGq5qhkFSH7qK6xq+zp2dJ6BQxOXo3j9AVSfuUQW1Sv3GAmgJJuV61vl9UIV
                3Kh1ciNTNUElD1mXi9iranGp7CLqczKx23srd0JDRU5j2pXlYI7PnZOll53W9A79pdKNZmTozQ2w
                YuClsWXKZEDv0zIdNITiq9BwDKIzCvP3bTLomhTLoiasdDoVIn1uxstGK9YJrAjWHG0w9lodyLCG
                Ip1Wv6kyNaVmwyrajUivBXEx0QiPDccdvRLwbcqr+ChpBD5IJYCSRmHKxsGYuX0Apm97BNO2PIwZ
                23ph9s6+mLenP95JHogFKXQr/JzL33N29cWsHY/g9bX34o3192Hq5of5uzfm7u6PeUkDlGkaxumt
                zGf+pt54f0N/zN3cH29t7YvJOwfgbULty9Sn8f0X/bBh3K1IuseFQxEmnNIG4kCIGltvs+DzJyKx
                4LuH8dbugZhKt7Toi4exZkg0cqItOBVixOEIK3Z00GDLPSHY/swd2PDxUMwnHGem9cPMxEcxel4f
                tIyxwRXnahasEhVYNb2tf6l0C8EhsNLYsY8xkiIA4/QpljCs4T4Y4aNFaKAGLZqKhb8jeRKDQRWI
                CMLqee7L9XoH9hoJK6ZteywWZAisGH9pVFNlakqphFUaG9ssNogCq32EVa7c0M5YTlJZud17IW/J
                d6xL1c2GVcP5P6mV8kQFeX1dJWrpqmrllXdyQP4yp+VQeflqUVkF63s5CmqPIu3sciza9DSWJr2F
                bMKKCaJy643CKckgm9n9+2HFla6uKCFwKpkEsmxFl3Fq5nbsvH0uNradhsynlwM5RFK53JUkSWIl
                s9hK5aWMRfRVxTST8t60Kq7ZZXFUAip5gSPdY11VHUqKC1CeugvbQ9tiq4+ZwUPA6Jm+EVZZtL1Z
                bPma2plNKc3EVowWP1NHR8ZWM83kYIvGHa6zElxOvG9zoYdWD1uAf5NB16Q0Jmi0Bpg1Qejg2xrT
                GNibzR4GDQOHZcsiqDLFntPqN1WmptRcWLmYAoa7DIgI98ARHopbH4yiFX8Vi/YMpVMaxLRsMF5Z
                2YOQ6oVp2xs0nTCaubMP3t7dD7MJH9Hbe/phFiEk/acnPoI3Nj2IN7c8hKnbeiq/Z+7qowxvHGdG
                Ym9M2yHz6oVZ/P72ngGYkjIQb+4egHc29MP6jx/B9oFR2HGHDTvvcyKJ7ipx3G3YOLEzlk3ugk/e
                vRMfbO6HORlD8Xb2o1iYOBSfffoQ1ky9G7sn3oXMZ7pi35MdkNQtGLvv82Lb+K5Ysro/PkobiE+S
                x+DV94bCQVBFNNNZ/fOwouMWV0xYSQqYTgefanFgA1PBpwKM8BJWPkztmoyHJmRQG6BTqRDGNHA0
                G8MfxEGZGL+WYCSZ2HjKso1M56imytSUUky2hsyAUE0LsmE/YXWQ7j2XsEpX27Ht1u44vugL1qV/
                DVasiIQTsx15O/UVD9H4kBSR1OYSnMexst3YduR9fLLlaazOmIWDF7dzLiVKOimHuf53wUo6kvd8
                bSkKJI3Lq0dmzw+xJew1pPX+Cud3FCKvhBRWsFRMFsm9gWV0UvIpd3o3nOYUZivbjyt4+Yqzkq1T
                euk8inZtw7aQttjcyohkAmAfd0paCEHFnZ0t7ogB1RxJ5c+Qx4voOK3Rzd9OBib769nCmd2YyWC5
                U62BLuifgBVtt9zUGsxgvIuQe4ctcKLZi/2aUOxlq5zJ1jOTFSCTLWFTZWpKErTNSwM9iIsOQUxU
                BFyRYej8cAy+SXkRHyczzaNbmrNzIB3SI5hNyMxLGYJ5dFtzUx7F7KShmLV7CCFE17WLjuVHDVb6
                yzhzU4ZzvGEE0S/HaxjnbQJxxt7hmJ7LtJDAm7m7L6Yl9sUHq3tjz+wHkN3BgpRYLTYPi8cPPzyK
                b9YwZVw5AB+t7I1Fa+nImGq+Tze2cEsfzGMZF67vhw83DsEH64fi05VDkLFkCJLuDcUOdwC23h2G
                rcsGYUk6h9NZPTetB0LDLASUs9mwamo7N6WG40fcV0yv9jIm0swEiJwcoWvZzFRwotqKuCAN/FXN
                v3TBoDZCLTfAE1b91CosoYNKMYchkxBMMZiV28cyuOxU0z/hrFjOVK5bFt17hlrAGopcuvdcOYZL
                0G7u2A1HPljMesS0rpmwkltjamrkcgM6IxoHgdTVyY4csmlUOevyqeo07D7xKR3Vi/hs81hsz/0Q
                J0v3EFYFnKZcuUpAzhw2t/v3w0pWuqICFwme/Eqmgmn5SGbqt94yEZkjf0DleeAoRztbX4DSynOo
                q6SPohOrUVRG98RcuJZbRDYgxxMYy5UQAnnlFGn5RVzctRXbw9pjM51VMnfUAaZWKTYGlNlOO918
                WGVYOA13cjZbtiyDhzBgMNKOp/D3Dks4XmF62E6lRoAqoMmga0oappV6OquwoCD0CFLjY6sbSQqs
                BKQCK3l4m5w+/+1h5WHq1zbOzTQwBu4oN7r0iMXncksEYbWQadvsxAFM5Qiq5P5YkPYo3kl7nNAa
                RVf1KN3REEzbNghTtjIVo+T79O0E0U5Om/EEx38cc5NHKr+nMQWTcabKODsILvaTeUzbPwJTDg7B
                /M09MHcTXRhTzQXr+2Drez2wp6MJOwmrTSPa4of1T+CL3U8ox8VmJg/GW+mD8OLunnhtZ0+8vvlB
                vLb6Xszc2g/vJI1kqjcac5JGI2vjE8i434ldppbY1t6MPSuGY8m+EZjH5T4+7UHY3FaExzQ/DWxq
                OzclORYkB9qz5Gwd4yOdzjiDjY0Aa7vVhdc5rANhpQoKbDIempJBZUaQSg8t08DuAb74mMvZbXUi
                g/Gbrjdhn8QGy5gqYPwFlP6e0gi6DE6zl+XMYsOYzXnu57rm0Fml0m1t6dwdRz7+XI6lNBtWtTQb
                NayPclmDpHByQaekc+KoBFANb4WWTKgYJ6qzsOvU51iW+io+3/YsVqbMQO75TSiuO8QxClBVe5EZ
                KJctj4FqZve7wKq6tBzFl2tQyPy4OPk0dt4xFWtCJiB95DJUnQbOcbR8Fr6kvICwqkR9BQElT1dg
                aqi8SbZKtkzDxhNYyYlR8Zr8h0vVRShMScQOTyds9bfS/srD5hwEAtM5BpZcK/XLgPt7yiA4pDXL
                1juQKbCyeAkGJ/YYwrDFGolxQWZEBqrQSt38VlPLllEeaesJCMQgBuQSWzghE479tOICqQwrQWWy
                KhcnNlWmpvTPwCoh1ovY6DjCisMeiscn28bjQzqfBUzz3t7RnylgP8xNIghShmF+8ii6oJF4m7CZ
                QTBNI4CmbO6PqVsGKN+nE0azmJItJKjeSSE0CIaZ24cow6ds6q+MK+PM3EFntXM4pmUOxZTsAZi3
                8WHM39abAOyL+Rv6Yt2HvbD5dqsCma0jOmH98jH4YfMYlm045tCdTU4bgPHJPfF8Uk+8sP0hvLr2
                fszj/D/ZPgIf7ByND7aPQuYPjyHzIS92hvpjQ2cbtqyg48oeRrf4KJ6a0VNxkpGx7n8TrBpiZC/T
                vnSbHHMUWNmxi4CRNP82uiTtP3GowKCyIoDuyp/O6hbfFniXDmiHja5eDg3ICR6dTTmbl0r31hSY
                mpTEMeGWY3ASWHRphGi2XNfHdd5N97f97odx7KtvWJmanwbW19VC3jN4Weoi/wlmpA7Ko/zKmdoV
                15/D+bqjOFGejq1Hv1aur/o88Vl8l/QK0k+sxoWqE0r2VCcHeOouEFZyYF5qdPO6fzusZKWrCJwS
                oljoizNV2DHoHayOfxnJvT9G4dJjqL1wWTnTV8UxZByFtfKfYqMo6fmLDSRfJBMsqi9Byf5UbI+8
                FdvVDCYGTBYDcA9z/QwCQC6wayrompRc7nAVrDKsEYSfGzu5wzdYo/B4gAGOgCDc9E/ASmc0w6jT
                IcI/ACMZkN/boxhEEdivEufHwLda2epZsJ9B1GSZmlDz00AvYsJdiI6IhysiArc+0EY5G/jujkGY
                v1NgNQBvbexLAD1C8PTH3N2DCYsGzd45CG8nDsQsjnO1pN9cwm4Ox5XvM7cTUEzXRDO29SOo+nNa
                ppgyjz2cZk8fuq8+mLOXbi11KBas7ocN7/TEpu5OJHZ1Y8/I27H7/Uex58NB2LSoN75b3AOfLOuB
                hZv6YO5Ogo7u7/0N/fDNMk73xUBsXDwY6z4aiKQZDyC1ZwS23WLH932i8MXSPngnsTfe3TQcb0wb
                gFsIqFiC+reGVaocUNeHsOI7kS2wsjfASlL5JMbebLr6u+Rxxv5+TcZDUzIwFvzUJjaCfohtfQPm
                0lFttTuVlDNTa1IuQs0ieFLExTUFpiaUaQlRzjTnMn7louE0Ai9dZya0zNimNmNXz3448cMK1NU3
                /2zgT1ZBaqE86kW5xBtluICzlftwqHALMs4sxdZDH+DDza/io+3jsCzjFew58QXOV51VbtWROcgr
                Vusul6COqSQX1+zu3w8rqqqmXjkmrpijwloUfpmOPb3ewYY2ryL1kfdwfkk6yk9dRHFtDdNBeTSX
                kuUp0/4IrIa1VDplU3GgHOor5KaqOrYP26I7I1HvRGaIR7koVA5Mypm8vdrmp1c/wYot0dWworta
                Z4vCo3462PwDcf0/cd+XluWQt5xE+gXgcY0ZK4NjsE+BlYWtsZVwbYBVzr8DVtFeRMk9gFHt4I2K
                w20Pd8AXO1/G+3RO7+4ZzJRpGKZslDNocvauB6ZtfggztvYgxAgvAmYu3de8PXReu/oSQH0Ip96E
                Uy9M3fRggzj+tC0PEVQPK9PN2NZTGS5nDGUe7215BB+t74lZa+7HWzt646UdffBm4mB8nTQWy+f0
                xM4RXXDwjjicCud+82ixp70Bm3uGYeUL7bF8ZX98zjTzE7q3L7/phe+fisf2rnQYbWzIaevA7jgN
                9nd2YN+AW7BtSi8sShyJqZkDMHP3Yxg3bxBM3lCmgb+9s/obWAXLpScCKxuSCav5dET3qDXK44Oa
                jIcmZFDZFVi1IKwiWlyPtxmDW+xyNprxqPnXYJVFF5bD8hw2e5DDGE5h+peiNSCNaeUmlRFJA4bi
                1Lq1RM8/AytxDXJwXWxCJaovy/2AJ3G8MBnJx77GmowZ+GrHWCzaSPe75TX8kDMNaRc/w5k61m9O
                UcXJG57oK0+1qm44XtXUYv5O92+HlXQ1l+ma6pjryqMJS0md7EIcnLkRax58G1/FTcTm++fh2KR1
                yF+ei7zjhSi9KC9g5IQCKZFcFVrLPLlOzkAwK+ZA2WzyHKtiIXx+AbbcM4jWOZ6uxcPKb0WikcFv
                MirPn0phMDZHWaz8mcHBSHGGYXtIMHaaQ7DD4MBqzvN9Rwy6+2lh9ZcXmOqUF5n6mdQINKihllsr
                5MZlucUmKIgKVKQT0b6bdFbE+PniRbMR65kW5mgMyNFqsCtEi80hauyyaZhy6rHbYlZOVScRYsls
                qeXaq3QGaaZJHk/jwl6jG/uovQzEvawcGXRt66wGbGJqsszkwht2DzqE2hHv8SKaqZ8nlumfNwzh
                EbHwREaia4+2+Gr3JCxKHYYFaUzp9gzBm5vosLb3xvQtPQkuARZhs41OKLE/5u0aSKANIqzkYHx/
                Aohg47ApmzmuSM4ibn2EqR9ht6MvoScujfOTcam5dFsL1vXC/DU9MJvQeoPTvCbp577RhNhQrHz7
                fuwZkIC9UVYcCFYjOc6AxG4E0cBY7Hn5LqxeNgSLd4zEez/0wbKxCUjpYML+CCMy2oVgZ2wQsu4I
                Q+rgW7Bhak9CeBTe3jsEk5l+Dn27F7TRYfDKAXa6yyiue5wzAhGuENzrsOIVu437NAwbrW6s4nbd
                RCctB6KzCCG5RCXTRLdkdnDbO5DC7b+HadkugRq3cxpjKt1gQgpd+04O3xni5P6io1cuYfDgk5BI
                PMz9a/HzgUarahBj42dq7E9ptWoYAnTQB1IqFZw+rfAs42WZWZxVGPawIdsZHIpk7tt0q4fAYnwq
                YnwwRtLYUKUy3uXarzS9hSBt0C6LAelc18NOJ1J0jDOdHzZbA7AlRIMvA/2R8cSzuLArRTnedLmC
                9Uu5Q5kVSipWozmg5E4b5fgUJTchFyMPeZU5yCnYgd1Hl2L93vewPHUqvkmSR8o8h69Tn8WK7New
                Ly8JJ4uzUVhzVDm3r8yS85IH/dbWChBF7Pm/CVZyWLxSHr5VcwlVVeSr3LJ98TJKEvOQNWMDvn9o
                Npa2eR3bOsxG2pAvkP3OZuR9m4lyDkcOcXSaW/ACs+JSObgn0BPjyS1aLxc6yFXsZFl5BXY8/wY2
                xN6GRA13oFaPHXoVdnCHJdK5JLKlao5SdEak2Bkc3hBsctAqsxXaySD+gbCa4opC+0AVjAyoAJMe
                rc0awkqDQL0GagacTq1msPEziAFIYGkolYpiEOvUBkT5t8CrDgNWGtUsH+FkMmBDmAUrQgzYajMx
                8KzYRgBtp3YwCBMJWdEupgJyVX4SW0tRMueXpgSpFbuZIqwwmbCGLe83bOlft7uRYDcizu1GbHQb
                hMclICzCAXd4FNwRHtzVow1hNQEfZQ7D/MyBmJY0EG9s6ou3CaBZW/tgxpZ+BBVBs30AYUVI7Rys
                SL7P3jGQsGJayGEzmA5OZ7o3XVLAK5J+MyVtlPSRKeJsOZvIz5lM4+ZznnM29sKUTQ/h5cQeeDlj
                AGYTmIt/6Icf3u6OjY93QNLQDtjweEdsHNEWGfdHIKsLYfJmdyxaM5jTDcCX73RH4pMdkDK8I3aN
                6ozk/lHIvMuFPXdHYP2YLli+dhjeSxlE6A7B6Bk94WgTgUjCKkJJBSMRH+pFuDMEd4fRWdntWEG3
                vDI4HN8S/j/oQrGDINrBbb1DYoHwUcTv2yUlIww2EwCbmEYls3FIZqOzg+nUBu6PLTYXnTedrkYu
                f/DiC0csHmEc2cRZqeXxLzpF8kwzeUxQo3TK224aFMSYUcubb/R6GIMC0FtnwPvcz7sMNmxmvKwJ
                MTGO5XiZA9sI2m02yiqxYmV5KZNFeZVXIsuUyJgV7ZDGmg1gpt2KPWYDtlhV2GxXYYNZhUU3t8Dx
                l99Czf7jCqzk7XWsVD+Bqp4G4HIt61oZSqou4mLpGeQXn8ChoiSkFKzA1mOf4/vMt/HFrlfwybbn
                8OmOCfg2dTLWH5yPXWcWI6d4PWrr6ZyuOLR/pH+m+11gVVJbgks1Jcrd2Er5JO2VW4Kyy3H2s2zs
                fGoJvrtnOr5u8wJWxU/C9rtnIOfxr3Bu5jaUf3MA1dvPoiLzIsqPlqIsvxoVJWwFLjAJLCxCcfkl
                FFWXIHvZt1j+4MP4MkiHrb5B2KnRYwt34AaTFRsZcM3RliA9tkpOT3e1XQ7QB9FpqUKx3uTFNLbM
                7TRB0MoLASw65fopPa27QSWvXJJPI3SUSm1EAO22DwOuld6AljpCLbAVXAHXYYLHhM+DWR6Lg4AK
                xw8M7B+Yau4wRiJTH4XtTD23GdzYaghj5QjGRlaQ9ZzHWs5jtS4Iq3T+WKHzxTqDERu1dqwNtOBL
                pjLfqULwudaFV+gUEliZ4l1hiJOH1NFZuaKc8EbEwBvpRbeeP8FqHoExdc8ATCasBFTzCJl39hBi
                u4f+HFCEU6Pk9xwOmy+XLiQPawASATVtez9M3dYX07b1U6D1thz32jMUszJHYPo+OYM3FNMTmQJu
                eRivbXkQryX2wvQkLo8p3sfbRuKL9SPx5dYn8PHm0fjunYeR3o0Owvc6rBkcjQVf9sBLLMsHm4Zi
                6caRWLV6NDZ/MwrZ7/XHXsIqLVSLHbe5sOarfvh4S198vG44pkwfiPZt4glsF7yxHnhiIhHjioQz
                Igy3eWwYF2Il3IPxrdWhXCawhJBaSheyNJSOJtiC721M181mrDaauK1N2KBj6qQ1YouG4vBNhMV6
                vR2rCKiNJicbtFCkMU7S9V58Rgfei/vM5u/PhsvCBswKIxtQszYEFkJRJN+lnwzTcR/eSEhdbzGi
                NZfrx0a2G0Ez3aDDejqvdaqb2MC1wnZtAJLUOqwinFYxplcRUKvoulZTjZ+rDT9pt4aNrq8e61pp
                keaKQ6ojEnvoHDf76vDFzRqUzf8EOFOoJHRyEbZwSh73UsaKeYnu6UL9AZys3Imcoh+QmvcJEo8v
                wLLUl/Dp1iepZ/DZ9vH4mrBakT4HWw99icxz23C8/CAugPWUc6Nv+hswNaV/pvu3w0q6hpc/1HIl
                alFGTylv+FGAJVeJnuPX9CLlVpvMsd9ie5vJWBf1IlZGTSK4XsLaW9/A5ntmIrH3QmSN/gonX16P
                S3NSUPnZHpR9n45LG3NRknQS5dv3Y/3wCVhkiaMzScDGoCh8HxSN77SxWGGIaZZWB0RjtTYOq60J
                WEt47PaPQpJ/HNZo2mGGuS26+wYjLiAYUWoXolThiAzywBvohjPAieAAB+18CAIDbGhFyN2gMuMv
                hNj1tOABKl/E+rfC68EOlseJxEAvdqtjsM4Yw6AjoCwupFjYn8G0nZDapmeLKi03g361XosVDODl
                +kB8p/fHt7TzKxiYa/RhWCmQYqX7Th2GL7RuvEZYtbXqER8mTwqNQFRsNLxMgyKi6LKiIwmrBMLq
                eSzKGI656QMwZVd/TN5IN7WlDxYmDcMH6aPwbsoIBVgCJnFZcsmASL5Lv7m7huD9zNFYmPEY5iUP
                V5yUQGoK5zGV0BNwzdo5ELOZYs5PfRQL00fifc77ox2D8CmXtXjNI/hoZU8sXPUI5mwg3LYPwbQ9
                j2F28hgsSHkKKz7qhyNdw5Hz5/9AItPBT7/pizd2DMaitYPwGcG2IO0ppnvjsGnXs0i8j87K4Is9
                HRzYtHoEPkiWW3xGYvKMgXBHepj+hcEV40FYbBQiI2PotrzoEhOKsU4bvqVrWc4UagWhs4YuZh1d
                img9YSAN3CZu483UFlb8bUyxtouY+n0VYsQndK+f020t0YUwVtxIJLDSdUwlDS58HeLFYDpnp09r
                BPnpEeRPyWdTujKsdZAZ/gSfnvMwqS14kOWZy/RTnHWSkY7I7I9dmgAkq3TYZA3FBgsbUDq/dXR+
                a6k1hO1qk42xZCPYrIqkQVyrIZBbmrEuIBzrg8LxTVAoPgh14rvRg1GwN5H1Ui4ykCTtPKvhCRyp
                yEBGwQY6p8+wcv8sfJPxIj7nPvkk6VF8lDQYH22hG944Bst2v4htue8gO385TlYkoxCnCDh54nrD
                ES0xaXLRZ1Nw+qX+me53gRWLRUzVMS2uQWltFcqZvMpFZPKKHgXpJQTWmQpUpRSg8uvDyJufhOwX
                ViDx0U+wscc8rOk6FWs6vo6N7SZjR8epSO0yG+t7zsG2R95F5iOLcWjgt0gZ+CmW3/4CvvAOxLrQ
                ftjlfhTbPKOwyfUotob1b54sA7EpeAjWeIdglbs/djoGIo3z2RX5OL6LfRJvO4fjbddozPOOwWz3
                UH4fgunOQXjLOQCvhvXDxLA+eDasFx539sAI50MY6nwQ9zq74V57JzxuSMCS4DuQqO6IrIC2yFa1
                wU62xNtNZqTY1Mi2B2KPSU1pKC12G3VMAQ3YyTQvkeMkitVnMEq6stvmQJItDDsNIVjF9GAjndgP
                Rro/uxcdxVkRVtHhEQRVDFNBSYfaIiI6mrBqi692PY8P04dhTprAagBe38C0jzBaQEAJqBYkDcc8
                AknANJOAmkG30ggrJT0knBakEWh0V+KeZhJMirPa2ldxV5ISitsSdzU7nePsHYX5acOxkOO9u7k3
                3l3fEwvW9sT89b0xb+MAzNtMqG0bgbm7HyfQnsCGhX1w7M4I5PjeiMSR7fD5sgF4S+5V3DgYn+0Y
                hQ9SnsZ7qWORvOUZ7OkRzv2sJbSisHnLE1iYSVe46zFMmDMAoRFOAqoBVk5COzYmAe6EKNwV48RL
                YcFYr7Nhq5yIUVuRQpe6h2BQJGf1LHKbS8Pv3dzmu1j5RTsJriTuhxQ2JinqEOxSOzg8DEl0t6mB
                GqQGqLGUDczTAS2R4PtXBOhbIsDQEoGGVsqnv64F/LQ3o7XqBrQI+Ctu9P0zbvD5T7Ru5Qt1Cx2M
                NzFVvC4AXVto8KY/HZw6GGmc9zamjTsJthQfI5LYCCapzdijMdM9mbFLSwfFcRLpwkU7CNUdLOsm
                uu81dIQrVGzMbiaY/Z34IiwSHz1yJ7asm42jRTtwujQDB05vRuKZRdh0bAEBNRPfMJ37NHESPt4+
                AZ8kyrPVX8bS7Dfxw4Hp2HX0C+TkbcCRgkSc5bRFtUfozPIVP9bwRqqGozzyOq5/kkPN6n4XWCmn
                72rqUVMtb6+RG5rrILdDKheRXa5HtbyGS26lEbdFNFfn1eB82lkcW7UfOR/vQtasDUh/6XukPPUV
                dg//BDsHfIBvey/A2nvfQ3Kn95EcOR/r2szCqs5Tsa7za9geNwEZMS8hI+4NpMW+itSIsc1SmnMC
                dnsnYkvsRGyIG4c9sROQ2fZlpHZ6A2s7voFl8W9gfdzbSIyfha3xL2JL/AvYTG1k+rquzSSsSZiE
                ldQPbSdiedvnsazdRCyMew7zw5/EkrARSA0fhQx7X2TZHkJmSHek2DsgUzljo8cxswoZViN/m5Bt
                MWOvxYIctvS5bDkPm0Nw1OzAcXMYTrIVPxEcgmNMJw8RYnvsVsiVzXILz7xgL26VY1ZOSf2i4YiJ
                Q0R8FKJi2jXAqkc7fLmzEVYDCatBeGMjXdA2OiamWkrq92P615ACCqRmbWv8pNhPYCSQkk9J+/5G
                iQ3jTE8dihlZIzAnZSjnLQ5NDsj3xDS5n5DzXLSqH75Y3BPfzL0fX8x/ECvefgi7xnZFzl1epCTQ
                Wb7UFR+vYDm3MfX8+mEs/agHfljYE6tm98SRNx5E5j0eJHZ2YMtjnbF9y5NcL6aeu0bimTn9EO5y
                MhXmdoj1wh0bgwQBdmwk7gt3YIrdhj1MwTIC9DjkZ8AxpmvKg+6M8mIFOw7xM5eNwgEqh7DaL6IL
                U168wPT+GCF3iOOnBodhR5wLW9qFYFt7Qu3WUKx7IA5zH4zGiB5edB0QjrsGR+KeoTHoPiwW9wyL
                QbfBUbitrxsdegQj/l4Tou/WIaZLKBLaedEh2oNodzDuoV71OLE6IhJp0VHYxhQ+0ROOJIebjRSB
                ZTchOaThAZMpLjtSvCFIiXAgNSqM8c7UOM6N7fEEcicnVnVLwDe3t8fSu2/F0mHdsWzOcKzfNxub
                8z7G5gMfYs3OaViS/ASWJD1FjVMOlC9LeQursxZgc+6X2H1iNTLzE5FTmIaCijxU1TLJk1fjiZST
                XVWoFUn+JHebXNG/o/t9YCUH7eTahcpaXK6miF3JaeV9sBVcsUs1pSiplSc014jJUuykXJZQSvtV
                WspxiutRkV+L4qOlOJuZjyPbjyLjszQcnpuKky/sRHq/pcgYvZz6Etn95+Ngl1ew3/U0clzPISd8
                AvZFjGmWclzPIss7FinRz2JP1JPIinoamXHjsaXNc/gw6gnM9z6DRa6J+Nw9EZ9y/MWRT+Dz6Cfx
                ZczT+DpuLJa2GY/v207AKkJqbbsXsKH9i9gYNxWJkdOQFTkZJ9q+zvkRgglPY0/7J5ESNQiHCK2z
                xo44p49Hjr0tDtrb4IgtDset0cizhqPA7EYRW+8SYwgqWImq2WqWWgwosqhxXt7iy7TvoByAN7nx
                fkg4bmMQx7rpJqLjYY9nBY2PRlR0O4RHReOuh9vhi0TCik5nbtpgTNs9BG9tGaw8JeHtrf0xdWNv
                TN/cR4GScnyK4Go8fqUcXGf/GXRQb2zsiTc3PaK4KTnQrrgozks0iwBUQEWHNXXnAEzfPRDzOe07
                dGhyc/Pk3b3xRgbdGZ3cN5/1wPrxt2D7PU7s6BKCtC5hyLzDhd33e7H+2Vvxzce98O7awZjxwyN4
                540O2NQvHKldncjoFIYz8Vbs7RyKLQMSsHrKw9i5kfuCqees7cPx5Kw+aB/iQnt5nlVcOCLoMNtH
                tEWkx40HQm2YRchnE1Y5/ioc8w1Cvr8OJRorijQWnFebcEZjxCmdEScMJpyw23EyzIHTXhfyorzI
                jjQjPd6G5C4erHsoFt8+0QFfvdQJy6Z0web592Hnt49hxZpn8dH6ZzBv3eNYuPEp5S0wHyU+h0/Y
                UHy04zk6zKcxd81oTF8+DG9+MxBjP6Ejf6c3xk3vjSGvP4hhE+7Em0/dju9HdcWeUXdiw8g7sGnk
                3dg6lJ8PhWMztaVnJLb1jsH2fnHYMagtdg5tj12PdsKex27FntGdsXPsLdgwuSuWLngEX381Cku/
                GIVVy5/Eum3juZ2exkcp47Bo+zgs3jAW3+0aj1Wpr2Dr/jlIO/ElDl/YjLPle+mczqD8srxOSy40
                YDWWbIh1WSRn9ZSHoNBGVdOE1DJjqie05H1UDYngb2+tfj9YiWtSXtnKH/IA5nqxW/XKX8OrTeVP
                rvoQSsuDT6tQUVeJcm6EasJNzlrIFQwyeWk5/yvhDEuYXso90Je4aQoqcHlTKkomvo1jEffjlP4O
                HAm8DTna27Df3qVZOmS5E4esdyHX2hWHTbfhGOeRargdH2nbo0+rULSnom5yILKFG9FBkYhRRyFO
                E4O22lh01LXBbYZ2uNvQAQ/qO6GX/lb003fGRFNffGhjOup4EgeixrOlHIfkdi8j7ZbpdH9TCMhX
                cDBkAg7Zn8JR+ygctw/HSXs/gupB5Fu64qIpAcVGN0qZglToNajU+eOSToWLGi3OM/XI1RtxROtA
                mjESi0Ki0VUuXWAK6I5PgD2hPZ1FNMIj2yqvqur6UFt8vv15fJD6KOamD8EMpnzTtj+KhXtG4G2C
                aMqGXj/CSiAlx66uPuguAJvBlPC1dQ9j8oaemLK1j+Kw5iYNw7wUpo/JwzFH7isksJSzhZt6Y87a
                R/Dxun6sFP3wPgE2J5XgyxmJT3Y9ipXz7sW2nh4kMUU63PImnAjwxb4YCzaMaodvWJE/2snKvn0E
                Fn7VA5+PikC6JxC5Ol+kWwOx39gCiX2j8O2sh+i+RmL1zifxMZ3cG4nDMHhOL8SFudAhPhwx8ZGI
                iopFBzfTwOBQ3GMzYzpdaZaD0As1ICNYjSNMvwu5Pc9qNTiiVyHbqkaSS4fENhZsv9ONHY/EYeej
                t2D3k12x8r2HseLbgVjBZX654XG8n/oEFmSMxAdJg/EVHeryPU9icdJYLEh8Bgs2jsbCTY8TTmPw
                /tYnsWjHM1zvcfg8+Xl8lfYCvs16Bd/tfQ2LMp7AJ6lj8FUKXXjGJLy2Zwze3jIc3xDUG1YNwOcr
                euOTNQO4vOFYvmY4vqeWrxmmaNnaYfhu7XAs5afo23Wi4VhJh7ksbRQ+SRuDT7LH48vM8fiMqfYH
                mwcp908uz30NG47Mw+5jXyCvKB3nS/ehuPII69sZWgh5xbs8WYF1TJjTUFVRVVmJktJi5X0J1azD
                chiHiVHD9VIiGZf6d6SA0v1OsGLp5fiUSMAlvwXT8vpoQkuedVVRVcENwByQ9hLybJwabiy51EHe
                wa/cMcnxhVhyLUgFpystRFm9vEK+Vsmaqy6eQMEH7+LofQ/hlNWDCzo3ig1RuGCJximLu1k6Syic
                M7iQTxXrvSgyRDMli8G7TL06+/rCFBSI1myNfXy10AeaYQiiVGYYVRaY2FLbVDY4guzwBNgR6WtH
                DNUmwIn+mli8T/hlhzyAXKaARx2DcML9NA5bx+FAyCRWwheZhr6IY/x+POQ56ilqJNO9AThlf5i6
                C6dtt1AJVCzL2ganDAk4pYnHPnM8DhvbI93SCZ842+MeZxjaR4UjMiEWjnbxcEdHwBPeBi5PJG5/
                IAGfbZugwGp+xjDMSpV7+0bxN6Gwh6C54qQaIbVAnk+VTBFqV0Or8VIFcVRyVvCd1BFYkDYS71x1
                LEuGTd9DsRK/w+8LmBq+vZtuK4NpXS4BmT0aS1YPZkp3HzY/0QkpT96O5KfuwOaXuuHrD3pikTzc
                b89oLNj1GBZtGIavP+mBba/dhbTx3ZAx7h4k09GsWdQHHxAG05PpXpJHKantKzuHYuCC3vDGRCCh
                jVdxVvIiibaeNggODcHtEcF4s3MUMnrdjp0D2mLzgCjsGByLjMc6IuXxTtj59C3YPKEL1rzWFd/P
                6o7vPuyBpUv6Y+lKwmDDSHy9bSR+2DIG69c+hdXLRxNaj2HVNwOw/v0HsO3127Fl0p34duwdeGVU
                e9ze34UufcNwyyMh6NQrGLf2DsUdA9y4b0Qsej/dCUMm3onHXr0PY2Z2xYtzumPa/L548bNH8cRX
                A/D6N/3x9dKB2LViCJfRF0vXEYSbR2DtpjFYu3EMVm8YjVXrxMWNwPcrh2PZD0OxdPlgfPvdQHyz
                dABWLumLNSz7ymn3YsWkO7DhlW5YPqEjFj8Th60LB6Eg4zNUnU9CRekxJjxySRANQh0/a6pQVV2B
                KtbHWuW+XNonoZJcLFpfxmpbzGpbRhclF5NeGcTq+Utg/Tu63wVW8rJSYZSsi9LJFwVeTAflWFZl
                DSor6KCq5aJPWkq5u7u2EnXKK+OvbCy5mVmBlnznDMqKUF5fgkJazsLqQpz5/jvkDBiE7FAXTmn1
                TJPsKLGE4pIpBIVGW7NURvdSqjfjElVusHMebqSFRmAWW+T4wJbQ6IPQUq1Fq0AdVGo1gjRqBLI1
                9tdRdD2B8ux1nbx6SwuzSgtrkBbaID/cxmHzbE46qFicMUbgoqENCvW34YzPnTiu74W97sFIjxmF
                I87x1Dgcco7FQaakBySVdT+F/e4nOM7jyPKMRqZnFH8PxyHHQBy290FGWC8cDO6BDMcD+MzbFfcz
                1blVHrrXlu6qXQSckeEEVTzCXEwR72uDxY2wyhyO2XJTMFvxjzOewseZT+DDjNHKGcH30x5TDrYv
                JKj+RuwvcBIwLUgfqZwVfDdjlPK5MP2xH4ElLksOsL+dKeMO57RDCT6CMG0I5mZzPntH48OU0fhk
                ywh8LiBYOxpfrx2FzzaMwkfb6EjoAuYnjca7BNaniaPx7VYO3zCCbmIU1qwYgxXLBuGbpGewMGsC
                pmc8i4V0EQuS6fw2D8HoWY8ob72JjXHCE+2GOyKSbjMBRncoOt3qxWSmUxlTh2HLu33xwwcPEYQP
                YMnXvfDtt73x3fL++H7FIKxYTUisGaroB35fvmoQvlsxEMs+ewTr5vbAjpcewA6mZ3se7YLMwR2Q
                2yMGRzq7sK+9F5vi3Hgx3A5TqBp6eyA0Fj8EmXyo1sp36R/sNcEdE8xGxYmou5zo1M2Le26PRZd7
                26H9g1EY0jsWi4Z0RPaYO5E0uhN2PXULdo29DTufIWSfpp7i9ye7YOeYW7Fz1C1IfKwTdozogB3D
                2ytKGtIJ6T3bIeP2KGS1dWN/Rw9Smb5uizAi/e5bUfXFp8CZU8rZu9LqOpSzHlbSRFSxblXVEFys
                i3Lj8mUCjMRqkJIQEmBKjZZrsRS/oVzsKW6qnv/J7TvyBNF/R/e7wEqK3nhxrDDqR/ByRS/TLdVW
                1RFa/OQA2RTlHEsuc6jgZ0O+LJtILn2QV8pLosgJBVx1tagquYSKpAzs7f0Ysm0xOOynx2m1BsUh
                wSgwmVGgMaBUZ2uWanUWVBoIK053yWTFWasTW8PC8XJoKNxBvtDrDVCrrdDQUamZigVRAQYV/Ewq
                +FrU8LNqEGDVQmUhpMw66KggXQvcZfbFYqcd55wOAtSAMgKtJNCI8zeZka/y4nRwGxz13oajzn6E
                1QAccg1Frnsk9nueQnb4c8iIeBmpkW8gKWoq9kTPQCbTx9zIV3HQ/TzSIp/BAfdopHkGY3H0g7if
                6V7nWFbOtpHwEFhhXi9c7lg43eG45a44zPxCnrE+AG+tHog3Vg/HlJWjMJ/pzILNozF/82OYu5Ep
                IVOJGXQ901YOxFRWUPmczsoq/WauGYJZTEdEM9eLhmH62qGYtmYwpq4ehCmiVXRP1Fv8PnXNIMzk
                97mcxwLOayE1n5q1ajDeWjcUr3N5r24ZhTeYMr225XFM3jYG0+iWpvwwDFNXcf4rmF5+NxTvLWdZ
                Vw/FZJZ56sqRmMGUZ86WJzFzw1P8Tehy3DnLemP6R73xxNjuiAgLQaxXLoh1wRXO7RHRFqYYD269
                Nx6Txz6AtI+fxuaVY7B83aP4lK5lwZq++GhNf7o9gur7wdj09UDsoHPb9fYD2PXqnUgcSxg8noAd
                /WKRclcEsqMd2KvT4SD35Wm9HheoQjVjTyfPJQvFDF0wrGoLNFqKzlvrz5jxNSDAT4dAfx0C/LXw
                99Mi0FenPFW0RRD736hBi78GoNVfb0an61tgamsVDpjCcJTzPanVsmHT4IA6AAdUATioCsQh6og6
                CMdUQTihUuEkdUrEhvSMRV5zH4a8IAsumhzMFhjPQQacaqlDxvUmnBrxAs7uykQeq+HFkiIUlZcS
                WjQMQh/+k0p6mSahXmyT/FD6CaLkT8Ck/FS+N3QcF+K8CpkEyQ1zPw35rbrfBVbN7RoOusvtkQ1Q
                +6VkWDU3XqW8goMbr5q0v3AwC4cmPI90HxMKWmtRbw1GmcWMMgLnjMmIUyYTLtI1NUflgTZcoqM6
                H2LnpxEFBNgqczBGWOUNNj4NrwVnYOl1eujkzcuU8ublq+7z0tBpaTQ/SeV7PXrRXW22eOjcPKiy
                hKHUFMzgtqOAbussXVuh2UxHZ8IFsxPnzR7km6NwxsxUz9wRx6134IitOw4yfcy198b+4H44FPYS
                joa9jUPBbyKFKWQW3ViiZwwWxgzE7VG3Ij6uM8Lb3Y7QtrfA6ZJXXclrtiLQhqlhl9s74u57u+DB
                nneid/97MWDIQxjy6MPoP+RuPDKgi6LeA277UX0G3o4+g+5A3yvqM+h29Orfher8k/p1Rs9+t6Jn
                35/Uo+8t1E+/ZRzRIyJOL1Lmw+Up4veeV6T0V+ZJXTUPmd+Dj3TE/T3b4+HenVjW21mmO1mmuzg/
                usqHO6Frt3h0uMWLmLgQdHDTXTnljdJxaO/tiMj2Cbj/7rZ4q3dnupB7sWVcd2x58T7seK47kkd1
                RfLw25Da/xakP9wWGXdGI6u9C9mRduwLNTLd1mKfLgjn6L6Lue9KdFaUaUwoV5tQqjWhiG4832jF
                CWsoDtnD8Ckbu1j/QLS0aNiwWWD1s8JAUPiqVVAbGRdGNXy1KhhUBrp0uQtCy3gywJ/fTYRS+8BA
                jAvQYG9YLE5z/oUGNS6xISzi8puj/BAnThFYxzmPCzYbjrORPGE2sfwhOOnvQJKrHU6+9iZw4iBA
                WNVWVqGiqh61YrXkLFcFcVNejrLKUqlqPzkNdnKrTJ24LiXrkeNajaIlkfvklLOB/9dhRVTXCM3l
                +y8ktz7KY1MVyYbgv3q6qvPr1yGx2/3IVQejiI6nymyle2EAWW04S2CdplO6aAxulsqD7CgmrPJD
                rAw+g7Jjl5rsGGDSQ6vxg5pBpLx5+QqoFFhdBSoFVleBSqT3vRH9gwKwzewlkNyoJKxKuKzzBGG+
                3YkzwcGEFYOeACw2MtAUcIbivMmNfFMUzhJaeYTWKUsXnLDciWPWu3Ay9BGcCh2AE/ZeyHQ9hP2O
                +7Hb2R0fRNyOO70etI2KRkRcG4TFt0EEISWgiowKR1R0OKJjIhAbF464NhGITyDA6L4SmC4mtAvn
                d2+D2nmVU+k/V7iitu3D0b5j5N+oXYeIn6lte1HD+D+poX+7DjL+T2qYR9TP5vf35inli2vj5ifX
                k79lug4dY6g4tG0Xg/g2kYiJlXXl8NgYRMoNzN54tHcmINoTjocjnJgZ5UQyt0VyGydS2oYhPT4U
                2VHB2BsZjAPeYBx223E8zE7Ha8M5VvQLjKlCgYDewvTdxk82agIrrRnlBFYDrCwoMNpx0hLKxiUM
                X3I/tg8grMxqNnJmWARWrfQKrDQEldpEJ05nLnc+qFSMFcaOXi93P2hhMGjRJigQTxI0ac44Ojcu
                36hFMR17EYHYHBWYQ3GGLu8c3VSJPRRn7Racs3KYKRQFvsHYo3Yg/aFHcPHrLwimEtRWy8mshnNf
                ymXtlfWoqSpDSV2pwikFVKyI4qhq6b6qqpnhMFVsOBMoz13nCKIfD1799t3/KljJixSrmdqJw5LV
                FTYroOIGqOWGkM/G/jKkNvcQ8qbPw8bgKBxn5S4iBEpMDCqrma2JFedY+c8xiC6wf3NUqrKzVbLj
                rM1CmOgJCCcWm2x4iEGl0wUqLkpzxU01F1Y2v5sxnNZcng56Se+k63MQSnRvAisGdZ6dyyasyjiv
                SwzSSwYdgcngZ7AXEloXjC6cN0YwpY0hvOJwztSGFSiO00bSfXmwL8RD1xWBlOBofOSKRDeHCe3d
                YYgKj4IrKoaVVt6XJ4/3/UkRke4rciE8olFuXP2adm+4W3nxqMfr+kkel9IvKkqe6tAAwMjInxQR
                KU84+EnhET/N70ex39XjyDQybXPn2Shl3sp3jhPB6Zj+Rl15I3N0VByio+MRkxAHd7y8Wp7OKqwN
                2jg86B8Sgg+tIThmdOAEYyXPaGaM0JVrmJZTFwmfIkKomLrEFK5Ma0U5VaG1oZK6KM6bukiAXOI4
                JQTVJYEV3VMB+582c95skL7l9zsCVfA1BUGlNcLMNFDvQ1hpVNDSWWkJLD86dT1hFaSiY2fsGNmw
                Bolr12sRHRiAEf6Mm7AYnKQTv2jUEzRaLt/aLJ3XskFkA16ssaEsOAwXgu0otNlRQViV+Qcjq5UR
                ic4o7H/6aaCogMaoQmGUSA4wX5ZrIuvlcqIy5dCL4g/IIuUxxlckKWN13WXlAm/lnFnDpKynMpPf
                vvvfBSu5sr2mRoGSrK9I3FZ1bY3SX4DV2F/uvixduRZHeo1EYisbweLBBVrwixaChq3ISbqrfMJK
                gktavObokobgYKt5mmnZWY0Bh4M9mM953B7kD72BLomBpNHqaNk1zYaVJ6A1nqK130NYFerDcIkt
                70W6NUkxz5kdOMUAypdjZHRsxQYVpSYwxe7TaQm0FFtPiBrlRAFTRlayi4TbRaMKBdpA5FrUOGI0
                IY2V5ONgJ+626tEhJAzRDHKXJ46V36nc0BsV7VKgFS3PuKK7iomJogOJRizdhygmhhU8Kl5RFCt7
                FCt4ZEQsIsJjFIWHR9OhNCgyMrZhOD9lnEZFRDSM++M0HNfrjfpRyvQyH0oZR8anZNqoZs5ThguM
                ZHwZFq4sh+6J6xoRzrJHtOXw9oiJ7tBwRjQ+Gk6BFZ1VO2c4hjqc+NzuovPg/qD7KON2rdGFAKzY
                UAXjstqOOqpGTTipCSqqjCpRWxSd4744w1iRY0AX6JgKdUaK+4Owymfs5HF+J01hWKEPxoNBWgQa
                AxHEmDEGmKDzYxrI1E/Pfayj/AkruZ80IEiewKCFmbGmITjVjAVXgD/6+wVinSMSR9nAnTcQpITY
                eQFRM3SB61KskvWjkw/m9HbGEGOtmqlhnToUJ1obsMdXh+QudwDZaUBpkXJsWK5xJJromioJLrmF
                hqkgvzGvoXGqRU1NrVJPf6yflJgugZQ4MHl4Q6UYLH7/rbvfBVa/vB/o70nspJyFaHymjnTyvaqq
                SpEM/7ErzcfZKW8j294Rx1uG4pg+hJARt0JHRft+gA6lwGBjyuXAWbqm5qiIQSut40m2tifZymY7
                wjGZwIjz94GBqaCRLaRGo1XOAjYXVgmB/nhRa0AqK8d5zv+CNZgVheVkGnGO8DlpoZMjrC4y7TxP
                q9+oC0YdQWv4UYUM1mK2vCJJUQs1OuU6q4P8Lq8zT9N78LE1CnebQtDexoofKserOhFQBFUMnVaM
                k6ASWIU3gCommoolqOIRG0PXEd8ebRM6KUpo05G/RR0QH9cecbHtOI6oLSGQ8DeKjmrzM0VFEhqK
                ft7/l9PJ/Br1y2FXTydqnKfANCZa5sX+dE8KXLmcyAimvIRVhJef3gQCrA3XLQoeyk2wyaULMeHh
                6B3uwgKmyjnOCOQ6HTjiCEF+cKhyLLGKjUclwVBOyZnkIrO4XjoU7qN82U8WG06aGR9026eZap3l
                vsgnXM4TVucJq3NMu87oQ5HHRmmDNhSDVAa6pAAEsbHSBRqgDRRYBcGgZ/onZ4/leKfKDP8gOivu
                TyvjVac3I5CwsgX44QG/ACxhg5lrdRKOdG6EnpItNEMXNSFMVV0otrpx1krHZ2FZmXGUM/7AdS1n
                fTnmp0VGmBfH585B/eFcwqhagVW5AKdGXigsF4TKu2jEb119bEok9uunPEf+l6uLyqvrUFYpT1xQ
                ev+m3f8qWAmxa+igrn4AWGM/5VQqYSX9+B8q0nfi8JDHkO0ThiLquNZOW2+lw7LgOHfMARNtPQF0
                ibA6w8/m6AJ34DkGzHHC6hhTgMRQL57VG+EkrHQmE4xqg/Kq+CB5JEwzYXW7KgjTGMhZlnCcYxnP
                MXDOmhnoTC3OMrCP02WdZjpyjoEvaclJBtpJwusUlXclTRGYnaPy+V2RgRVMw/UKDMY+QyhydR4k
                G6PxYXAM7rQFo21YFCI8HRAW2Z6QoqMiqMRZRUY1pH/h4V46HS/TunB43BFwM32MYBoVHR1NV8V0
                6ipFRkZeUQTHaZA82O+n/g1qHHa1pP/V85L5X62YGHF0Dfr5sv7+PCX1C2eKKpKUtCFVpTyyThwe
                LtNxPZgSJtBBRijXmfG7O57DPLg/woGpHgd20WHtDLVgt92IDLrww3TToiNmC45Sx6jj3P7KPqFO
                MbZOUse4v0QnCKtT3Gd5hNVZEffxWbrlM4RUni4MW/n5pNoMs9YXgYwBdZAeasJLYGXUqWDSaZjy
                ySUwJvjLY5A1eljZgOk5H3+6agPd/O3+flhIaGbaPQQjgUm3f8bIZTRD+YytfKMLp+kic7kuh1gf
                TlAF/H7JFoLyECcbRBsOWx1Ierg3kJjIHFBJ+FChsKjhdFctcVUlB82VS4f4WXUJdWWFqCq5iJqy
                IlyuIt5+hBfrp7wyTw68/xu63w1WjU8h/IcimJT36zMXvnzl3Kh8ykPlpX+tpIJUDV3WmY/eRU7X
                7sjxseNCS+4gXbDS4klKlGuk26DlLaAlFwCdJoiao3MGQktgxSA8Qku/MtiFoXo9zAG+UBMSRraO
                GgaWXFvVXFjdTy1koOUQVnkE1GkrRVidEVhpQ3BEgaOFy5eACsFhU7CiI9RRtuJKxeB6ybqdahQB
                fIoV4rhaXnIRgn0mJ3bZwvG+04s7wvRoE+5AeEwkwuLkmJVbkXK8KlKOV8mxo3A6j3Clcns9rMye
                SAVeVx+/ihD9eGzrqmNeV+bRoCvD5XiXcszLBe9Vkv4/TkdFRcnxs0aFE1hywL/hU4Y3Z56RUS5O
                w086Ri/X0+0JgcsdQhcZwvVwchouh4CKZYqbwOXIMS8vAdaWsIokpB+OCMPbbifSg+VlD/KmYjP2
                UvvYwO2ncgisXMbQITrqI3RMR+mwj6uNOMl07WSQgQ7ehmNMA48TVicYE6c4npzEOa1nw0JnlacL
                xWkdQagNo6O2IVjTGgEqfwSptFDRYfvqAgkwFSyElYrxE6gyIYCuSmBlIdx0Ggv8jPJi3EC0DfTD
                G5zvHpsHeWyU8gm2H2PgV5THBu2EyYFcuwOZXKccOvMjhKqU+bhkHyGhKGBsHeXyNlhdqP/0cyDv
                DOtZvfLaAyW3YxWUhK9GDIIkNbROddUVqK4oRVV5CWorCSq5FvJHWPGTv+sFalcc12/Z/S6wahJM
                TUiuZldAdUUNF3NwBiIOq62uQUVZOS5dLMSBJ8ZgL9ODg6zwJ1rKdS5yXYldCbJ93Nkn5QwId9hZ
                upmTBFFzlEedUmBlxWGC7nO2Og/Tkqsl2GjDjX56aOQ4BAOtubDqwwBdzHTvEGF1SiuBQgdF95en
                IbAIm8MM/qMElbTUBzneQS5XdIg6rMCLQBNwUceYkoiOmNmPZT3EVjjZZlHekJMY4sC7dAx3eAKR
                EGVGRHwYnG0ECg0HrpWD1+KOrjik8HBKAVaD3KzAUuFd7lACwEG3EsZK7mwADiFyNWgUoP0IEELD
                6+T4ojAC4yrJ759JANMgmf4nODbMr1H/aJ4Cq7h4SWc5XoSU1w6H0wpHmLUBWBxfyhtDWLchpMPp
                Jr1c33aeOMReSQMXhrm4P7zc9mwIVHRQdNEHmeIdog7TPR3hvj7GbXtCw30lgAgy4nSAEXn+Bpwg
                kI6zUTtOVyKNjFR+RYyZk3rGkc5BOZFETWcD6lS3JKx8CCQ1Y6gBVhbCysZYUev0HGZi48cUUa2H
                KYCfQSb4Ep7+ejUigvwwVqXDFqZypwxhKFATkErj1QyxgTzMGMmm085k43hQx2yBYDrC9ZKb5PdT
                xwjCI/56bL7BH5defh3IyiZv6lBB7rC6KdVPnJZcIynVUZ6mICmikiZSckOJHEyXE2PKcax6Gg26
                qroamUIq7W/b/e+ClZLicQIFVkQ7+/0IK0qeFFpcWISzp04h/fZuSOeO2B/qwIFAPYo0dFLcQUcZ
                cAdsNgYQA0dDR0KdYHA1RycJKpFMe9jswgIG5J16uUJdBT86ITn1rAlkGqhv/tnAoWydl9L5HCWs
                TmjZutlNOMYW/LTASkUoMXU4zECXlvqI/icdZjmOcPk/ievGIBQdMtuQSyjnaPXYY5U0xoodhPO7
                rlB0darRLtymHJ/yxMujUdqw4spxHzl4LQe3I1npPXC5nAgLC0VoaDBCQuywsbW10mnYuO2Cg4PZ
                P5TDwzienAWU1KvhrJvI7XYr/UVOJyFHhYWJwn4mmYfM6yeFcFk/KZStu4wTyn3ocjbM7+p5OpuY
                p0A1nO7I7WH5nHaEhFo5XyvswTZlPRyOUGVaN+cTT6BFyGvkmba288Yhhs7qEXcY5gc7uO0YN34G
                HG6hxXFfw4/7Xhqr01Qet7ccGpBjmefYcOSLTKE4xUZRgHWCjYw0akcJKmlsjnKao2wcj+kdBJ08
                StiFdwguj6oF/INawlcTpDxh1lcfCKs6CHYNY0VnhB9hFcR9qVUboPfRQUMg+nB/+pp0CFP5Ybhf
                ANYyFk8YXChgjB+la2qOxAEeYtzsZUMmT+44qaLrC2Scs3GXN4An0WXtpVs87s9MpLUOpx99DOXb
                trLKXVYusxIglVJFrHfn+VlAt3WaOkmQneHvfBG/n6OJOssR80trcIk0k4PrYsL+Hd3/KlgpKOeH
                Aiu5rUZwfjWsKipRWHAexw8eRmZUO+xWqZHioutgwBfQ9RSwhcxzOHHC5cahAD2OBtLJsHU7xkBr
                jo4z4E6IxNGwNZtJ0Nyq08DHoIUPW1l9SwaYwMrQAKrmwGokW7IfTG4ct0SwBWflsBtx1HoFVkHB
                dIYMKrbkknrKQX0BmiIG8nEG1HE9W0Rp6UVSKahcljGH8NwfZMYuBmS63olEUxTeD45FN1soOjli
                EO/pRLB0QUx0O4JKzpC1Iazk7J6cmaOT8hAMdFFhzmC6EjvMZovy9FMdt6GRgW7mNrBZwxDCVDjM
                4SVMCDk3p/VEUzFXFM1+VyuKkPhJjlAP7LYwyvmjgu0i188UwpSsYd6/Ps9gQllPt2uke7bQHQgA
                HQ4BnJfDmdK6G8opahNFZxjtgpuuMj4iDl6mm/fx91Su+x67G3vYQKQwxTvA9OvcFTDl0cGeYuU+
                QR212HGElf2QzY6DVC51nG7spBwf1VkbnAn322EC6xDndYgN3mF9KN0LYaV340N9GN3RzQqsfLSE
                lZkgugKrYMJKewVWKk6v0xigay0XEesUWPlYDbCr/dG/tS9WmlyMDzdhFdIkmJqSgFeOh+ZaCVeL
                E/l+wbjgE4JzWjf2O8KxmzDOplM7HWTHabqrbbd0Rup7C5B77Cg25RzFl+t244PlmzHzy1V47p0v
                8NS0TzDglQ/R44UP0O+VRRj02iIMfvldDHt5Pp54YyHeWPglvlqzA7v2HsXJ86Wsz1Jpf9vuX4aV
                HF+Slx1KV8XPqtpa2kJCh7/lvfeXmb9erq9GTXV5wxMTRBwoj62S6zXEX9Yx5S0rqMPZQyU4nJqP
                7O3HkbzxALaty8K67/bg+6924Puvd2HXqv1I33QQqTuykbJxOw58/DX2eROQyUDLors6zAp6nBVX
                seisHEdZIQ6qLThMV3WEASSupDnKNZkUGBwzhSHFGYcxah2iNSoGk4HuysR0UA89bblZrYVNxcDS
                qhFgZFpIeGnUGuiDmB4SWPICiUAGmo42/gXOczPLtJ8VNZfAOUhLflSOdxA24voO0UXlcpmHBEQE
                myJWAtERpTJIWvKTxIUdUGBlR47ajp2ESjJb8c2GCMy3R+FOezA6OVlRvW0Jq/aQM2YNp/nlMgJx
                ReKqCCCnOJVQgoqVnbrrrm4YNHAo+vYZgDGPP4VePfugTXw7QswGu93BcemmCASn08N5xShOzeuN
                JGgERpwPgeH1NBzcFnDI5Q/33fsgRo4YjUeHP4a7u92rDLOY7bBaCEiHm86KAOO8ZR7hhGiMlDUq
                lk5OgBOhLE8+Zb5mTiflefCBhzBs2DAMGTwEcXFxDW4tmNDl8l0uli/Mw+9M/TiNpL7hTHvDIyI5
                7xiER3twH9PCNwjr7aHRyDZyn+jkTLKToIrAGZMXeWYPTtPJnLIwnbOwoltZ6ek6jzF9P2aTxiOM
                +47w0DrpoMRFhXJfESJMAUXHRLoQ7KXLWs793K31n2EOvB5+ej/4mxgfbPhMKhW0cg0W46klgaHS
                BkPN+A3w1SAoUA01401PsDno6rsGBOADwnwvnXmBhsskDOXMt+ioxLYiOi7GxDEBlDg+xpQ0vBLT
                hwS6Jq6LmhBWEcIsfy5jMYtgzmF9OcplH6RjWx0aj496j8Zrb72HHtMW4f6JC/Dw2Pm474lZaDvk
                FcQPfBXRPV9BzMOvoV3/qWg/aBpi+01GVJ9XOWwybn98NnpN+hgjpi3Da59ux/Z9p5FfIpchsa7L
                Fe01pUwRK5TLH8SXKCgTWPwTTPuXYSUHvOVAtxC0nJ/l8t4/UkhuiakhpOrqK1mwctTK2QMWSCQn
                FOTkwYXTlTiaeR7pG49h05dZWDJrO959YSVmPvEt3hzxJSY++jGe7fMORj40G8Mfno/XB32FmY99
                hVnjP8G7497GxvufQZYjHgcYGLlXiwFy8Gopx4DYn0HTHGWbdYSGCYfphNaHt8eDdG4OVRAM7BdI
                qx5kNsGo0SHUX42wABUCNIFoZdbSshsYbBoYAuRaLB1UBg5TtYIpoCWmW4zYyXQnxR7C+UsZGVxs
                nY8y/ZN07yABlmOyYL+JZWDQNUc5IjrGHFaKREsodunCsFbvwdt2D+4INaOj1434yChERDWcdZMz
                a3IAXdK3xvTsajkcDvTo0QPjx4/HuHHj8Nlnn+HZZ59F+/btCQmzksLJOJJeCehkXnL2TuYnqWPj
                OI3DZNw77rgDzzzzDGbPno1XX30VY8aMQZcuXQgnSTltyrQyXmPaKdMJfKS8UsbGZUnKKt8DAwNx
                //33K+V66aWXMGXKFDzyyCPKmUSZh8xLJPOSFFPm5yGgvEx9IwmrqFiCNNqBeyMceJnzXBcWhb1G
                Fw6xIh+lczlJ2J8wMFU3euhIXEqDdUweeqi4KzYUVjYcIsbVEUJKJC7qMKFxmPv0EPfJQbrcBlmR
                xelWsSx9Wv0VjqAb4WfwU84EWsSNBwbAz98frRkzfnTWOlUoHbuVMSNuS8NxtLDr9QjluJEcd7LD
                g51siM7ovMrjgI4QkCJZ9iHGvSxbjn1KPB0Vx8cG7xB/NxU7v9QB1o+9XNeNxnjMju+Bfo+Mg/vJ
                Kbhl3EL0fP5TDJr0OXq++Cn6vfE1hk/+Gk+89S3GzlmFp+etxmOzV2AQ4fTIG0vxwMtf445nPkX8
                0HeQMGQOnlv4A7YRWEVytF7eSFF1gdlSCRlQ3WBYBFJiWgRYzez+ZVjJ/UH1ysWalcr9egqkqGo6
                KvFXdXVy79CVMwX1VWK/UH2hBvk5pVj/WSZmPPM1Rt0zHT1iJ+Bez5O4xzkKD4SNQ2/XW3g44k0M
                iJ6GfrEz8UgsgRUxD4OckzEg8nmM9ozC7OvvwU6VB/sZJFcrh6nc34rDGDzNURZhdZCWXF4M+YO3
                De5lCxiiCoSeKUegwfgjrEIIKscVWLUmrPwIKw0DzxjQcCxLpZc327RCcKAPZjK9koPfSXQfGWZ5
                Qy4hStAcZqt2iNDKoVPaR1DtpeT13s3RPgbZPgbpPgbrNmsodrC1XM00YVawB7c5zOgQ7kYcYeK9
                comAwEqOOTUFK6ng0k8quUDh1ltvVUBwzz33wGAwKONI/8ZPmZfAoHG6xk8ZLvMQST9xPy+88ALG
                jh2LBx54QPkUIMoxLwGQwESmaYSVzEMAKP0FfjKscVxJ94KCghSQjh49WlmnQYMGKdDq2bOnAkwB
                m5RLYCifMp2bgJLUTw64R8fRbUVbcGeECWO9VnzlCsFGm7wAQoskK/c7U/Bcpt85WjP2sbJnsbKn
                M6bkDd9JBNpus5NyMUa02GcOxF5zEBsfNTKsWqTKG2/ouuSprTu5/ESuz3q6wg8j2+Fepnp2xo4P
                U/pWgYyPADonfx86KB+mfkEIYZpvU9tgZCoqFx1b6ahs3O5mrQ4Gui97Kx88Rye00R6N48Zwxgtd
                tYhQ3E/tY8rXoIY4+imeZXjT8XO1cpkqHmd2kmqPxfI7B2L2C7Mx4dt1eGdjNlYln0FybikyztSD
                yQ9OES7nUIvztEP5rOMF5MApKv18OVZm5GH+snQMf+VLxD/8AuIfGYs5S9bjYF4+x5A0qpQsqKRp
                oaH5vZ2VLKm+rgqlcuUrYVTHP3l8XmWtPDFQSnPFTgk9yaoTu4/hi2nL8fSDb6FP/It40DMRD3kn
                YkDbV/Bi3/lY/OoKrH0nCRvf34c1i7Kxc3Eudn5xBJs/O4LtCw5g/fSdWP7Gcnw7egZW2e5HKi17
                Njd2o/YqLQQrMNOin0v625qlTLOegcBAZYv7hSsGXdUqWNWB0DJ4gvRGqORaK8IqmKAKYSAFaAPh
                Y9IiwKBna0lnFaSFRi8H4IOgDWqN8KAAzLHYGLxO7GHApVmcLGcYlxFKV0iQElz7WO5sllFerirl
                b46yGGBZBFYWgbXFFoKthNUKoxvTQtzoHGZG2wg3omPofGKaDyup8OKkxAU999xz6Nq1qwInAUbj
                NAKBRrjIvAQIMkxgIvORcWVZ8rt79+54/fXXsWTJEixevFiBVefOnZX5yDgyD/kucJIyyrxkeimn
                LFd+y7L0dBgCTVneW2+9pYC0T58+eOyxxzB//nz07dsXAUyVGl2YzFOA5XK7ENrWC1e83B7kRNuo
                MLR1G9EzxIDXrGas5HZMZBq2k6n8LmMAki0+SDH7EU6BSGVqn0YnncEGJYOpV4Y6HJmqKGSpYrCN
                23iDy4l1TKfXuCOxwhOHZUy5vw3vhK8ibsMXEXfis4humBt7P8a064UEb1eY+DvQS4V0hi64LQKt
                bvgxrQ1iQ2UJCobGxwR/Hw38GVM6vQ5WNpg2wsbKWAxVa/CkLQzLTB7s1YbRsZkos6JMbudMi1VR
                BuMsnSlr2hVlcf5Nxc4vtY+wzGWsZjEWU3oMQfJnS5FeWIwT5XW4VM6qS1NURtaU0yDJXYKl/CuX
                10NcPsdfF4ihYpTXlqKQZiSPQNuUlocJs76B94EnMe3z9cjJu6hc/iBZlly3JTgQKZD6PWF1mblo
                VW0ZEVXNYlQTWvL8ZZJT7oYUQ1Vaj3MHC7BpUQrmPLEET3Sbhl7REzGiywy8NfxrfPnmdmyly9q7
                PgenU48g/+AZnDt1EWfOFODSmQsoPluE/HPFKDpWhKLcPOSlpSF70QKsJUiSrQ5kstJerSxufOWN
                yrTviiwNqVeWWXberyvdbCRMbEgirBaEhqMTWz4jpdEblGtkVCYjDFotbEFq2INUCKSt9zfqCCc9
                dAIrtV65VSJIF0iX1RrtaesXME3bScezx+ZGqsXNcjYAa5+BMjoIqlBCUiQvWW0o/68pg+uaQUDL
                m3c3Mr3cbHDge8JqSqgbnZxmpoAuRMREwklgRV+54LIRVgICAYtUalEjrKSC33333fj0008VUAlE
                BACNjkVAIN8FagIbGX71vBrHle/ijEaNGoUXX3wRTz/9tOKCJk+ejHvvvRcmAl9SNwGTjCvQkrLF
                xspxtQhlnlIeWYbMW8An08jnQw89hCeffFKBqbi2BQsWoF+/fnSxKgVsUkYpq5TRG+5FSDuub5sw
                RBNU7SLdaOcMwcN0uS/YI/ClLQE/GKPxnS0K3zkisNIdjhXuKCz3xOJbwudLwmcx4bMoqhvei74X
                C2IewPyYh/HKbX0xodtAPNdtMMZ3G4ax3UbimW6P46m7n8ITdz+Lx+9+DqPvfh4j72VD/OAkdLpv
                IiIfehWeBycjvPvL1DiE3PsYrN0GIbRzX0TF9YHN0ZmuPRI+xlD4MpVTa6zQMj00MKVz0Kk/ZpJX
                rlmRpjHTyRl+fFZ/JsHVACx5u7dAyk6XZ0eKjbFhaTp2fqlsowkHVC2RJY7y4QE49vUy4qfh5Q8N
                1y4wU6opR21ZCaqrmEGRNJfluFNFMVBdSPLIY3rpmDj6ufLLWL5zP4a+9A7cDzyFaV9uxN7ThSir
                rUUJTY084I9Y+AlWckPhP3Eg/r8BK+Gl3E1UzW+EVn0lXZU8jYplqLiMysI6nMg+h2UfrMezD7yH
                wR1m4NHbZ2FS/4+xZGYiMtedRuGhClwuYtHLiW+ufHW1vLOsjOwu4XyY3yocL0WlHImvJxSLj+LY
                svfwrVGPXbS86ay0VyuDG1+UKSKoGiT9ZNivK42VTCCwjbB6k61ZLGGkodSElVpDEMmBT50WFlYO
                C9OSQF0QXZVOuV5GL7DSEGgGOitNAGx+PrhLbcKH5jDssnmxR96mS1ilm1zINDrp3kQCXHn7r6SI
                IVRD+X9NaVxXebV4GmG1nrDayPnIG5nfIqw6ElZxhJVXLgqN/TmspDI3AuZqCRzE3QwcOFCBVWMq
                Jv1FjbCSaQUKAhb53TitQFBAJcBrPEsnoGpM0zp27IgZM2agV69eyvBGtyfTy/wEWtKvEVbyu7G8
                8l3mJ+5KjnmNHDlScX9Tp05VnJaklgIzcWCNzrERgGEJTqaCDkRze8RFRyEiPBK3hrfFwKg78Wp8
                T7we3wPPd+yJibf2w+u3DMcrt47AC11GYvwdo/DkXWMwovtTGHT/s+jz4Dj0eHg8Huw5Ad37voZ7
                +r+J7v3fQvd+b+HeflNwX9+puL/vNNzfZ7qiB3rPQO8+UzGs3+u4f9B0dB48D+0HLkT7fguQMOht
                RI6YgYjHpiF+yFTc3nMq4u95BqFd+kPX5h4EONvCX+9F60A7fPz0MAUGYqA6CJ9qNEhlnKXLpSpX
                QJXNfbSXIMtmXchSMgPWA8Z7qjVY+Wwqdn6pLLOFKWUAHSTd2j29cPzTrwgUeZjAlcM3NCOXawik
                8iKKlVt5Wh91icNKxXrxk5X+EqvxjsP5eHnxKtwy8mW0H/wS3l2xB7n5rM81tSi8VEgHJpbmaljx
                m5zxb2b3L8NKubFRgZVYuwZYVTEFVKwdP07vL8Z372/CiPsnolf7tzD6/vcx74W12L36iBxrk4dR
                oa66DOVl57nO51HNjVJWW42CimoUyeX6ctNkfQ0ucJ4F3Ci11XWoKjiLQx9/hE//rMIO5vhp3EGi
                dNlJV0lalUalc4f8cvjfUyp3nLxGfi1BMpYB4NIFIIBACmSQqFWEEtM9nU4DI4PHoApkfxVdlQ5y
                X5eBsNLrDAgUWKn94fDxQS+tHZ8RInssHiQRVClmF8vrJLDCCEURQcXWNIPuKoPLTW+mUgmrVDrA
                FKaC6+zB2MB5LOVy3gh1ob3ThNhIpwIrB2ElLqYRBI3AkQp9Naik31133YUnnngCr7zyigKjRojI
                dAIiAYA4FgGSzEf6XT0vGd7ogmS8xx9//MeUUuYr4JLUUOYrMJRxGkHXuCwBjEi+yzxlmHw2jvvo
                o49i0qRJyjwFVpIKCsBk2QIrKdeP68lUzRPtIBQdiJLbi+LawBTfEfaO96DNnQNx/0PP4K5e49Bu
                8AQkDHkDXfouRGfqVgKlU//5hMs8tB00l0CZg7ihbyN22CxED5+Fu/rOwAO9puMhQqZnzzfR5+HX
                MeChVzD4gRfxKGN95H3PYdR94/F09yfwfPehGPrQGNx571OIv/NpRN3xLDzdx8LZ7wWEP/oG2o2c
                he5DFuLOUVzmYzMRM3ASwujW9N67cJPKi/+8SYVWN92Me1vfhPc0TE8JqBSmfOlmgZOV7tyKHL3t
                ihqOZcohEcky0pllNBU7v1SGhelisBHJamYUnR/Egfkf0T3RAcnTPgVWrNdiSJTnVokTohFBEc3F
                pQrUXKpEWVkdzvNn+rkavLV8D7q99D4ih0/GsMkfY3XqMcVtCaDKqspRebnhu3INgcCq8VrKZnb/
                Mqzq6uVAmcCq4UUP8r9AUnkSMUG85vMkjHrgFdwS3BtPD5qJ7z5NxNH9+aiiixLXVFZ5BiWVZ1l4
                eaUPC0JIl9NAKT5RxA0gaybzVC6Ild9MEQveXYI1f7JjTxAhxQr7M4m7uiLFXTW2IFf1/0dKYQDs
                ITiWEyTD5WCn3h+tDUz1lDM4hBUrhFZPKGkDoVMHElQqJe3Tq3UwBdBZ6U3wN2gQGOQLdysfDNM5
                8Q0hkmp2KhJQpdFJpV+BU6ZRxHIqASblbATXP1aDq7IhlcG53sZU0BCKZeIGQ1zoEEZYRTTASpxV
                DMFzNawEMk3BKiEhQan4HTp0UPoJQGS4uDKZVoAgDktgIuMLBGWY9FOr1YpjknmLYxIXJFDr1q2b
                kgZOnDhROR4m8xAJrGSeMh8Bk0wnakxTG5fV2F/m1XjcSg78y0F2STOlzFJOkYwn08q8RcpyIugY
                vS4qAuEx7WCOvxWOjvejQ9dheOTB5+jKJuKuAS+hy6BXcHvvVwivl3H/w5PQ+4EJGEKoPH7nkxh/
                22i8dOtIvNHxUUxtPwzz4wbg3eg+eD+6Fz6KegifRdyLr8Lvwnee25hGdsJqVzusdbbBMqaWC4Pt
                GOv14DZrCOxqpnZBdjZwofALi0RgTEeY2nSDN7YHwu97FrFDJ6PTmGm464npuLX3RDg69EVre1u0
                UFvQ3tcfrzMeN9gd2GlzsuELYww5kM30P4fzk7s4RAd0wdhPaO1lXEhsNRU7v1Qa426XWYftAVYk
                trsP+2d+qFS6WqVmXzkOLT5Lyf/EZZxEfd1p9rmEi1WlSD+Zj8VbMvH4vGWIHzEVIf1fx50vLsaK
                1BM4SbslVbf+ch0qayr4/SpYKaaGv+QSgWZ2/zKs5NEttZe5EpeJKhZGrJ0st6zwMvbuOotp4z5D
                r3bjMfT2yVj97XacPJKHyjLxkELqS6R3IdXwFEJJWyu5VtUCJFk7+VFCr1nZcFBOTKlC4fwClH78
                Nbb/ZzBSVIQTK6wog6BqUEOr8pMIAoEAgdAcpXCanXQ9S+h4esuTFQx+uJmw8pMLPgPk4DmhJY/3
                IKzU6gCo9GpoCTKjXHvlr4XBaIafkbAK9EV4S188STv/PSGSTucjEheVzuWkSzlZ3iyWPUvOQhot
                ynd5HG5zlM5gTGdrmsbA3EBYbWK/5VzOW8EudHQYESe3s0RHIEx5DEwDrBqPM0mlFhg1Siq4SCDU
                OI5UdOknQBEJJAQeAi8ZR0DVpo1cZBqpgETUOI3MXxyWuCGBmRxU79SpkzKtjCOfAij5LstvhIyA
                VJYjn41AbASRTCPlaoSlpJUCVVmGODmZj5SlcXz5rbisKC+i6ariPJFMjdsQXvHoGH0rBsbfh8nt
                BmB6Qn9M7jQAUzs+goVx3fFe7D34KOYufBZ1O5aEd8ZyTwfCpy02hLXBltA4bAuJwWZ7NBWFLbYI
                bGN6n2h1YhdhlGRh48G0LM1soPPR0fVqsJHOe2aoCffo/GD3uxlafx9oAnyUY5r+Ji0CmcIZDWEI
                dN4CPdPRyB6jceeI19D90alo3/tlBN81GvpOvRDnaIsxoQlY7GyP9SFctj0SOy3hSDF6kKV3Yb9W
                rsgPUS5n2ce4yGYsZRoY07+Im6YkhxR22XTYycY/Ma4b9r42F/VlFQqo5Ly+1FbJ+hquTJeD62eR
                V3oae/PPYHnqfrz2+Tr0mLQQ7Ya9jltGTcHg6Uvw/pZcnCyuk/e6KJcqVJUXorT0gnJ5kxgzARZn
                yy90JVKvm9n9y7CSZQlo5CZHuRhU3uFQRVdVeKYWn8xeg5H3v4nhd0zFZ68mo+B4IVM+ujDlYB3T
                RnnhmPJUwSszEvFnDVPDSoLqsrzlRnLk2mIaqnL6sCquYA0uXzqPoi+/xea/NMAqQ29VlKnsHIoQ
                yGpCsuOao2SCZAcd0GeE1f1BAQgy+uEGYxB8NFroCCOtniKsNAy2IIGVgIuwMgVdgRWDz5ewCgj0
                QSRhNd4QidUGl3LWLlNaO6oBqiyzAigzA8t0RRZk6kObJYGVrHe6zoaN1mBsZr/lBiem2J3oGGpE
                vJegiaJziW6AlcDlH8FK+gkoRI39G8EhnwILkYwn8BL4xcfHK98FIDJvGVemk/Fkno3TCJgEZo2p
                XWM/kYwv81TAwnkJiBqhKf0ax5VxBHwyX4Fc4/wEivJd+st00q8RmpLKRsmzuiIi0cYTgXYRUYSW
                C91dHoxzxbLid8Dy4Hg6oFisdUQixepGqpVpupWuxcL03BKiXCuVzZRrr9mCfQTRfrOJrsX8oxr3
                XRZdZabIZFCUQWXrDchSm7EwNAR3GwJgULWEWucPo0ae5e9LR+6HAI0/5JFDrTiej80DQ9ytiLiz
                Pzr2fB5tB0xB+OCZcA2dhtguIzAoYSBmx/TEMmcCVoa2wUZ7LBLNUXTX4cjUuZBNh5XNxiuLqWEG
                Y0uJ6V/ETVOSjGS3XY0UOrg9MXdg3wvTcLmoiOZKapy8C6EOhazplyrrcfx8GRJz8/DZ5my8+dVu
                DJuxEnc88yHiBk3DPc/Mw8uLVuKH5BwcKypX3ronhLhcTWNSeg5VZRdQSXAJwOS+QqXO/37HrBpg
                JVyU6yaqSavyslqcPVqOpwbMRM/2E/HG8G9xbjfHYVooqZ28ZaumlGCTdE/wKhMr0KKlulyKmtpy
                FFVLUsm5M0cW01jEX2coOQFaXlaEM0uW49s/syVTscLqpMLKAcZg7iTuKGlVKLHBewkp6f/jsGZI
                LPE2cxjepwO6PcAXgQZ/3CTOSsPUj2me8khjozgsFQI1ARz+C1gxSHwNbDEJq2imgZOsUVjL1K8h
                iASmFgaSmZ8NAZ5lNLCsBraGRuzVm5GlYwVplJ4p4hVliAwhhFSDMmU9ue6ZWis2sUJtZqAu1xNW
                Nhc6EVZtPE5EsfK7WOkbj1k1wkrgI5VfKrSoEVRXg0nAIRW/Mb2S7zKujNPoehrTQxku4JLfMn+R
                LE9AJCCTTxlPli/TybxkHnIBaOM4jSllYzkbyyPjiOR74/LlsxFIMh8ZV+Yp81IAxflIfylTbFws
                YiOj0Ia/20eKw7LiXqcBE+l2ltpt2MB9udEciB1WDcFEl0tlWtmYXFE6G4I0OterlUsYHOa2l3s6
                c7lf93N/ZMuxR0n1LS4kW7xIsnq5n8OxVxOJ9x0x6Mr9LS8X8bWboLcYYbbqYbBooTIxpszyhIUg
                +LPx82WqF2D2wBp1Hxx3PgX3wJmIGbsYbfpORc/7X8Okrs9gYXxXfEDnt9h7K5Y6O2JtSDtstcdz
                HaIpL7bbwrCN6eJufkq6uMfsQDJjOpUZQxrLmUpApYmYQqYylUwy27GDsNrNerQz/jZkT3oT9fnn
                UFJUiIJLRThWeBGZZwuQeuA0PltzCOPn7sb9T3+NuP4fILL/+2g/8hM88sp3WLAyE/vOFKNSnsOu
                eDGpw6zkNSWs68X8XSHoUl4v/yOslE/50rzuvwErSc4aLp2X4+HyTrHy0rPI2nUIg7u9g94d5+CD
                19fRIVWiqpSZL1F7mQCqrSlkqnqGZZSLxQpxuZYrJoQVeMmxKmGXLEBWRMROssMSAdrFC7j0xQ+Y
                /V8WJHHDp2iN2KnSIzeMgcGdsldtw/5ACw6orchly5gTyryelSGDLWNzdIiWeKMtBC/YzHD53QQr
                Wz6jiq1iED+Z8hkYWEY6IC2X21rrBx+juCsVx9HA0koHCwGq0ZpgIMgSdD54uY0HqxwepBolWIKZ
                JhgY0Gq23IHIZCXJMqkIK50Cq306M/YFsRVXWZV7tjI0VqQRRsl0ULsJuUS6th0s43ZqL8c/qjYg
                V2PCVrMNm3QOLGXKOdkWjnZ2M9o4nIhxRsDrakjjmiOp9AIA+S7waHRBjQ6qESYiSQHlU6AgABFI
                KHDgp8BG+jeCS4YLSGQaGS7DGpcn/eVTlif9BUxyHErGk2XLMhvn2QixRrgK5GS49JdlXA06WQ+v
                l7CL9SjvDYzk/NtHJiAywovu7lC8TthtZoO0TbYrG48kOl05htgcKYcW6MCV443K76uH06k0ii4t
                JSQaG+xxeJquJUqjhr/DDA2dmpVgMGn0CNLQodNt6TQtYFLfDJu6NUIYayEqHRyEnjv6TsR0H4X4
                Aa8iYdTb6DhmFu587Hl0HvAkbu8xCg/cNxoj7nkCL3QdgzdvGY4p7frhrYSH8EaH+/Beh874NCEe
                i+iwv+B2+iEiFmtCI7CGoFrDlHFNsBsrPOFYzn2xktv+/TAn3r/zbqx4bSqOp+Ri+TfbMfe9tXh+
                +vfoO/EzdBk5F9EDXoa37zg6qefR/dlpeHb+l/ho/W4kH89DAdOqSlbi2vpSVFc3vt2GEhjJdZcN
                tbqx77/U/Tdg1XAWkFmd8pJDXC5HWdEp7F6XgQFd5uLRrh9h2dwUJS+9XFvD8eTh8lyd6gqmqVeu
                bK8jvJge1lWTUDIPxWldWaFGWPGHsLqE80fxRZQvX4/3dLHYzZZBUj/l+A9bEQmU/6+9twCwqzrX
                /nu/7/57b1uIjJ05bjNn3DUKIbhLSHEtUtw1JMGtaLFCoVBaHAoECSQhxF0mE/fJuLvP5Pk/z54s
                eu4wNOlteqVfB97sffZee217128979prry01st5FNUVTm84yOuNy2n9sx/ph20iF8ikheCPBEB8x
                HAG7A36CSO9y+Xxu1oZBThPgcydYIzpGU0VF6Wmh3QZXlB1eOqHT5UHIFonjbSPxQl4Wvk0SCN1U
                bQGrN/RSfzbV2xjMipuIGQnH44Pkk/F26ql4M20yfp/2C4Ynl+DtlAu5/Bx8kjgZX4ROxaz4E/Bt
                8GjC6ggs9k/AgrhC1qY5+Ib2blou3k/Iw+8S8nE34VSQrjareORnqX1n4GnegZgBi4GLICHAaLlA
                ot+CilFOBkoyAzGjamRar3yVn4Gg5gUTpROYlKfZTuvNPo3q0jKt0z4NmLRM+Zv1mle+4etkSj+a
                sMrWR0553ONyxiIrOwsnEVb3h9TOl4g5DOHmefxYZqlehkUH0VYFU7E4KYdheh5ut/tR4KSyoqKz
                ++MQdBFWDo81ZpXDFQ2Xhsz2UJk76EPRMfBE2uC1eeD3pyEu43CExp+BxOMvRuGZN+GiSx7Faefd
                h6PPvhdHnvMATrjwcUy68Emcdc5jOHvy/TjrjHtw9qS7ccWku3D1qbfishNuINBuwk0n3oJbj7oB
                t4z7JW4ZeyVuPuo6XHPi9bjiuGtwzegrcc2pd+GCC+7H6Vc/gTOnv4lTp/8Rx971Bsbd8BIyL3uC
                4ejjOOH253DtU3/Ao+/MxkdLt2DpzhrsrGc01NWLjt4ulmsKCnUz6lPb9H8WST/895+Glbot9Iil
                JMleQYbz7Q0VWPH1Olw88RlcQ1h99uuigc/6qNNYf4c14qe+kS8l1sPzkukbiv29GtKYGWkl87Le
                IxS0wmDVSFjtbaPMnLcEH0w4Ews9VFKs3TayhlhCSCxnzbaK6mo1Je9KytslDOsWEFiLOTUN3Puz
                1a50vMX4/wpKfH9ENJ3HZ319xMuazu3xwMla2Ml1Xnsia8FkxDi9GOGyYSRrR8l8V1wQ+shpcmQk
                Jo+IwrsphGo8FZLfiyWBNMyOOxx/SpiEN5N/gZfSbsTTmXfgsey78HDunbgv/27cNvp+3Fl4H+4p
                mIb78qbg0ew78FTmLXgh/Qa8knINXk/+Jd5MvBKvpV2E32acg9+mn41f5U3Gr9NPxxNpJ+P67KOQ
                ma3B9jTOeiayrZFBh4aTKdTGVOBNeKh5gUNmICYYyAxwtFz5GEiYdDIDCy3Xb60z6zXVNiZ/k05T
                gUtA1HIdi9lGx6RtDBSVPtzMOYQfQ0ZGOkblpyOngOvy8jE2ewwysjJxfFoCpvGefEXf+JL3dBZD
                r0We4JD+8LeYQrBv6ZtfBbJwR6wXhQ4bIkJuxDAk9LkC8FFZKfxzuWKothIRy0oy2hFAVIwTMTEO
                2G1OOB1u+lsA3rgUxKfm4ZiCE/DcMTfhgaOux83H3IgrTrodF0+ejgvOfRjnnvMwfv7zB3Hmmfdh
                8uT7cO7Z6tf2LE4760mcet4zmHT+s5h09lOYNIlwO/NXOO3cp3DsBU/iqLMfw+RjHsOZ5/8Gx/zy
                t8i97iUk3vg8jn7iQ5z14me49OXPcN1rn2PKe9/gtdmrMWfNbhTtaUE1y66GkFHblJqd1CYNihGr
                O4AK9f8kWA0Md9pmNehbsNpLeDU3YOuSEqqqX+Oyw57HH6ctAPboTBTD8uyUjGnVft7KMLadIOuj
                opJS7Opp5jkqJKRmY2zZ3a/RRbXBQIRYz+17uL67eAPmXXs35qgbgJuwSqR6oTMsosJaFJdE1ZHM
                0CgJ3xJkC5hGkFDIeCC20J2Flwmsc53xcEVrBIU4a+RGN0NNu5NORNUUzX05o4IIRSXDFuvBCHcM
                RgaiYQs54UgMWp9SSh5uw3mHePBxcCKPoRDf0j4JHYtXUy7A41k3YWrufbg9/1e4Lf9p3J37K0zP
                fhRTc+7HNYV30u6i3Y1rC+7BdQVTcV3+dFyfdx/tAVyf+yDtYdyW+xDuyXkA07PuxS0FUzCNQJua
                cQ0uy7kQKQRWVv5RyCuYiJy8cd/BZKiCHV64pVQUWpnQT4DQb8FB67V8ILwaeFFZy5S3TPDSMpmB
                jPalNPqt/SmNlik//dY6cwyycAhp3oRzJl+lMepK2wtqJr3M7Mtso4b3Qn0so4DKkPmN0Sf0GRoe
                kxrCXXEBzGAF9ynv1ee0uVRXquwOpi0igGbFJ+JLXyrupp+MsctP7Iiy+ur54VEnY8LKS3Vl96cj
                0puNYY5EHKrB91xeVn4ugiwWblsE4u1RSGNleG5cAualHYEvU4/AB+lH4Y2ck/DS2LPw1MSLCbBf
                4O6jLqN6uhw3H30Fbj7xdlx/yjRcfto9uPis6biISuzCs6bhwslTcTHnzzvvXpx+7lRMmjwFN5/w
                IM4/aSouveEFXPv8x7jyjRl4csEafLBxJxaXVmNnUyuaWBglGhQAWcayqaf/6n418IMFWwsUakl0
                /B3+/tOwUgjYTYBYPeYFKymjjg50lvbj3ovexFlZ0zF18iuomF+L3jbTgYp/AhDTaxvBuL2jD12U
                Wj0a51md0Lo13Aw1W1/fwIMC5q8t66ncOqmuespLseXVP+KLxGzMivJQQRFM9gDmM4yb5YvDp1Q7
                H7B2+hPXfW0P4Vs34cU0B2IaueBxZyJOsLEGtKnDp98ac8itUR4p16OpoqJssXBGu5AUG4LH60Zs
                MAax8VFwBPU42ovhMW4EhgUwaUQO3oj7OV5LuAy/TroWj6bdhClUUbfkTcUNVE635N2De7Jvx+Op
                1+P5xF/itfiL8Ye4M/BG/Jl4NeEcvJh0IZ5K/QUezrgK07JvwB25t+KW/LtwI+F0Y85DuDXjMdyR
                /jCuJ+SmpE7BlJSbcWnmFUjOOAmZuScjN/9EFtIjWbgHlIsxFXoVbJmAYAq4ecomaMnUb0p9m7Re
                oZmevGmZYGEAZFSRgUN4dwQDPy3TenUxUBrtV3kYZaXtlVZ5GYhpmY5JDefKQ1PloXSa6rfOQ3nL
                9NvsW3kMAC0TY6SsqDAzcwmrTAI4Mx1Hp8Tjjjg/PnH58Sfe109dDnzl9mIefedg2jf+IL7iNZvJ
                CvVeVnBH2KnAvdGIUCdi+pSLFZ+d+w/qO4L0uWhHCkY6Qqz8gogKBOCI88IbdCDOE4V0VyQKaOf5
                I/FRcgRmhKIwyx/LCtmDBYEQ5lDBfZyShTfTs/GbzBw8l52L15Lz8bvEAryYWoCnswvxRM4o2mg8
                nTMGz+WNx1OsyB7OHoVHMwrxdsoEPJ42BrPvexjtO7ahs7sFXQzl+hXWdVBA1NUCtTWcZ/mUYKJp
                uHGN025BynpIxnlNBay/ou/UX/P3N8BKXec7rO+GadxmK7Zj7Aoqpk+eW84w8AlcPOYevDntY5Rv
                qYPOW81UXe2dBG8H+qnE+ginrp52hn29hDNPVONjdTFXysh2LlM4KLiJWU3cm7610d1Sj9qFizHz
                qOPxHkO0GbE+qqokLE1IwYKERDpIHB3Rgy8dQcxzJTMUTMZsb/wB2Z+oxqYw5BsXNQIj7BHW0LJO
                modS3eEeCTudzUFg+ZwupAbiEJ+gpzrRVGAj4YmJoXSPQ2RsOuKcY3GK/1Q8ln4VVdB0XJP3EG7J
                noZp6bfg8ZQrCafz8GroNILpePw+dCR+n3AE3oqfgM88EzCD9on3cHzkn4D3AxPwTtwE/CE0Aa8n
                TsBrSRPwKu3Z5FPwRPLP8XTiZNyfPglPJpyOxxJOwXVpxyIrg6EZC2dOQT4y8geUj0wAMDAIh1Q4
                ADTVcgMGrddvAx+TRvlJ/Qgwyk8QUp8nAwylMf2jtC+t18CEgp2Bmul2oLyVn8lL6QUiAyBNlafy
                N/2qlI8gpt/Kx/TrUlpzvJovyCW8NGIq8x6dWfgdrAaUVYB+MqCsvmI4OJvh2UE1HuccQmeuMwFP
                sjI9ye5AtDMSEVRMdo9v4BUugjJe7aIxLnhYQWpo41gqryifi+ZAjHcgTAzERiIUORxH2kbwvrvw
                WsiDT7mPeVT+Swm4RS51DE7E51RzHwcT8FEgAV8wzJ1Jn54RDOHDUCI+SEjCx6EUfB6fhpnxGfg8
                joCLT8c7/hS8NNKPGcefgeo/vgM0NrDA6XNbGq2XBU9KQU1QeuOtQ6OsdLBc6kn9vgdsJIFGWdF/
                vSyz3X09lthQ0T3Yf38TrLr629HR2clz41kpnrO+xQOUrGjGS1M+xqUTp+DyY6bgNw9/hDXzdqCl
                ogv97Ro6ppUQbqSyqkVbWw3DQJ6wiKTtu9SzirDiHnStrH+sbHt5vajmeoiskt1Ydutt+KBgNN71
                xFFVJVntDgt4A+f6ffiaN3wea6gVbg3fmoi5PjrNAdhHhN3tXifyYg7BcE8EIn3qoS6pTuVkH8Ya
                MZIqKwY+hwMhOlsw4IafUj0u2oG0qESkecYjJfEUjMo8H2fn/RL35t1INXQPbsqbgnszr8PzyecS
                Psfic18hZnvSrJ7nM/3JmBGXik/i0vCFV0O9yLiczvcVj/0rT4LVvvK1Nw5f8xxn0T5hePthIJtA
                y8HvEvLwli8frzHUnJKUh1GpcSjI1qe30pEuZbHvSzThqsrARzBRoQ5XQILGueee+11HTqXVNgZs
                gonpUqB8tdx0xlQ+BmoCiHrEa6gYvWysV220jQAl2Bi4aHuZAamBjTkms09to986Lh3f5MmTrVeE
                FLIacGmqvJRe0/ycFGTk8TrwOEdTQaQTVsen/rnNaibv4WyqqrmczvUdXJtHP1wcCGI5Q7vXnCGc
                56SiskciirCyhhuiatXgjSHCSu+RxsfQ6Ete9dPz2Am1WES4NLKHBuSzwTZiBApHRuGpQBL+yNBy
                ljsdKxzpWGNPxnICcYknhMU8p+XOIFbb/Fho92IOw8mvvX6GogHLf76lLy1ysqzEJmK+g9GEP4MR
                Sh6eSSzE+gefRt+qYlBWqbhZIZ+eew2EeRQaLNfdFBIq8yz51lRfvVHLtTqPyjT2isppu9Xt6OD/
                /adhpS703b0dhE0LwaOz4eEpblMXVVJ4+ZdbcN+1L+MkFtqTchkG3fQmls7YhKbdrehtaiO0mtHV
                WouWhmpeEG5vUZzbd+uTioKVWK1lA6bu/7oQneqsxX1WfPYxFt14PT4bdxjmx6diAUO/xbF2LPW4
                sCzgwRreII2EuMGtURR8B2QzMtNwW4IP2QTTSIZ1kepTRWfx2mLgpNpyayC+WBf80V74o/yU8PEI
                2ULIsmXjSO/xOCHjFzh+zI2YfMRtuP6IW/Bo3tV4nsteSzkH7yadhC/iR2O++r3Qmdd67VhLp1xF
                p13mi8ciP6EbiKXZscBvp1p00gHdWMrCtNzlw0pnAKsdcVhtj8MqOqRlrFVnE9Tf0Gk/cWXjsbgc
                HJEUwlgWyjxCKj3vz5/TMrAyBVkgMkAwykiwOO200/Diiy9aLwgLOEqn7QUWzSt0M90LBBjBxKgl
                rTcQ1G+B6rXXXrNeONageRqQz+xLQNLxaKplBqQ6NuWhY1M+Ol4tl6LSPq+44go888wz1qgLN910
                kzWSg9KY9CZfmUZbSM8l/AQrhoFadnJqEh4KJWA+K4T53iCWshAv96r7Aq/zQbSVhNU6qvzNjmR8
                7E7B1a4AArZIgsiFGPpaNNPoK0kJsU6EYg7l9FAkuqMRUAXplgpzIpKAi9bQxw4XRkbFoCDCjjcc
                VEa816vdudjqzsIWt17f0ogLcSjm/drB8LYiiv7v8OJbQnEuK/BvCKxF7jj6UAJBloglI4KYT2At
                JqgWTDwV7157DxqWFKG/vhXt3T0sZ3pcRmNo06NGaUY93W3NLO8qgXqjhGl629DGUFFlVXiT6XUa
                9XjXGAz/o2ClP42b3qlXYqy+C0SLXj7uYLzHn7Vl7Zg9Yx3uvOZlnJ57G05MvR5XnPAQXn/4S2xe
                UImuSm5i4ZvJ2xgGdhBHeoObymkAVlRbAmAnTemsWVGbl9EaJrURvYvnonz6dGwadyQ2UQJv543Z
                nRBESWoIZYkJqAkmooo3cied40BsZk4ubkpkaGIbiREEXgTVmTM2iECUD94IF+LsDEscrP1teciK
                HINsz5Eo8J2IoxPOx3k8x0sPfwgXTbwX1xJWj427Em+nTsYq/2HY4c3CVsrtzXSoTZTwm0JObIl3
                YkfQgz0sKBWuEGpdQbR4I9HMULOJIGtkaNJA1VjHMLbGnYlqdx6qXKNQ6RqLclcWyhwpKGXIWeTJ
                wdrYQnxrH4cXAofhhIRsTMgYjcKcwxj+jB8SVgYGgoQUkeAhGEgJadiV3/3ud9aYUSr8Wq91mjft
                QUZVKS8tNyCTGWWjtHp/T6BSeu1P7xCqE62gI5iZdwjVJmbCQG2v/IyiMhDTMWqk0AsvvNACqgYJ
                1JAx5rUbpVU6A0+BNjOLyiyXyo3HqjarHB7TGekpeCIhBSv8eqk8yeqft9WfwPuvDz8cRKOq2hWf
                gArC5Vve/7uofJKjqdBZAcUE/IgizGKosgSroDMCfvdIBDwx8FPZexkmujwBOAlTpy8OsfTNGG43
                1uPEG34XvrH8KBFlPM+SeC+Kk2OxOt2O4nQndia5UOlxYHecD9uS4rA9Md4a6bQiqE9vpVjfNVzP
                inYlFdo6VvR7CPzm4k0kQQs6KQLqWhtY+hTVDERAUJ+pvXqbpIrMarGabPZaQuLP/ykQHDANb2CR
                4O/y9zfBSgBRR8++XjLV0os8yb1daG1rQEtLF2qqgLXL2/DHh1bg2uOex5k503DFMc/ghjNexPQr
                3sAfnpmNDUtKULWrGt2tLRaoNFazwsA25aXYUJ/Z0INEXoWuvm60ku6iYWtXFepXz8eeRx/C8uxR
                1giPpSzg1SE/qpIDqAp6Uef2oJHKpImF7UDsk+R0XGJ3w/PvP8WPbS4cGq2ngiw8MZlItWchP34i
                xiefghPTLsCF2bfi7DF3Y9KYqTh77IP4xWFP44LRj+KsjBtwDcO931BJLfYehkpfEtromK3+AFoI
                zmYWjCZ/omXN/hCXEVJ03mauH/g8vCwR9UxXJwvEo5ZWw1q6hgWwKi7IaYDLOaUi2xzS6KOpWOLL
                xG+TsnBKGsOvvBSMGZ2J7NEDjeAGVirMUh7hsFJBV8GW6vnlL39pDZInNSQQGEhpuBil0TaCgvLS
                VPlovclT85pq/dixY61hkt944w0rT83rxWaBSCDTNgZyApKOU1Ntr+U6VuUTnlbwe/755/Hyyy9/
                N6bVpEmTLKAawAl2Rl1l5SYhieoqJSebsCIQs7IxOT2NYU8aigKZ2BhMxfZgOioCKWjitTyYVh9M
                QFUoGU3OTKqsPNzvTUZGdDQhpF7qfkQGqJgIq5CGFvI74Qg6YSeMnAzd1F/PRyXkdvph09DHdj3c
                iUKhPxrPJsXgqyRWrvEhdPhC6HHqs/JulMX7UEIwlRJilQz3agIJqI2jBUKoZ4TR7klFpysd9bZk
                FA/3WD3b17JyKpt+D/pKSq13cfupojp7KUCkO0QdhYEUHvroaTMX6Am9RSJO1KCsiKiXoWFvZ5c1
                7dcrdWpc/yve9/tr/v42WFl/ZCvPSO1OOguxtaOrjZKRsOEifQexZG0HPnt9FZ66833ccfHLOO+I
                6Ti94HZceszDeOiqt/HKtC/x3pPz8eUrCzHv/UVY8O0GrF1RgvKVVeheUQ8Uk+iV3aipacLuqmq0
                NLWguWgdtv3md1hwzuX4wp9LWZyBbRo/Oy4T20IZ0LjSGwJpVDNZ2B4qwK6kUdgTX4hdwXwW8Dys
                TRuF4sRR2OwfjW3eMdiYOAHPpZ2IcwMTkW8rQLJ3InLiT8HhSefg+NRLcFL6ZTgj53qcnXc7Li6c
                hl+OeQjnj52Os/Jvx5lZN+KUjKtRmDQZox1jcCXB9lkwFzucdFrWii0EZwsLfCth1UIAtRBUAzYA
                q2Y6byPX1/qT91kinWzA0WotSAVoftSwtqyO86Ih6EMDt6uks26PC2GHhsmhivxDUjomZiZibG4W
                RhVQBRUMvMunAm9UlQmzVLhNCKjCriFdXnnlFWuImE8++cQaLSG8C4OgpvSCiPKSCQqCzWBQaH+C
                lcZIFwAFwkceecRquzJpdSxKq21karA3ak1pZOZ4dYz6rWP81a9+ZY3ioDw1f+mll1rDw+gYzbFo
                qn5WudkpVr+zDH1tOncM8ngtzk1LxYtUVhsC6djNUHAPr3clVUazVZHs35pojaw89mc1tBLevzIq
                4+L4HDzHe1oQOQKRLhsivB6M9FG5e+xU69HwB9xw0UfsXB7r9FhvQbik6mkxLh8iGBKOdDqQxm3v
                pAKbxVC2nCDsdsWj107fYZo6y1foN/SpGiqyek4buc8GXwIr7AS0MlxsZTja4EjErhh9yDURO3Ly
                UXH6JNTf9wTL2VJ0ttaimeWXgQ7aFeQwgLHemGEwoxHmODsAKkukEE59bYRZM/ZSYFiP9pXAmPV0
                nzOEn97AMQERhdnAg0MBUeCTxlF6axA+KTopNKbRTwNGa9J7MGC1/z8dWEtTF7ZtKMOXHyzGs9Pf
                we3nP4fLj34cF41+HBfkPIVzU5/EZflP4PaTn8W0K/+IZ2+dgQ/vnoOF0xZi5ePLMP8P6zDj/ZX4
                6P3FWPDxaqx54jN8PPle1jST8Rv38ZgRmoQ5lp2OmaFT8VHoZLyfchreS5+M99POxafZF2FmynmY
                kXg2/pB2Fp4beyF+m3UR3o6/CB/4L8XrOdfhprybcV72Tfh55g34efbNuGDMNPxiwiO44ogncNn4
                X+GSwsdwaf5juCD3PpySdT0mpF2ECXGnYZx7ItKd+RimTqTDDsWVkZHWByLqnTbCykvQSBlRIckC
                cXQohne0ekr8eh8di85Vx6m17gCsydqGhUHfqPOGsJsKaEMwhHcTMpGflcNwZyzyswuRm/PnMEoW
                DitTsFXIBSENu6K2Ko0XNWPGDGssquOPP94Cikkv02/lJbAob+UhkJl12ofWK9QTUDTqqFSZ8lMI
                p33JDKSUt0zKyvTB0m+Tp45PCk/A0vZqp1IDu5YpxFQblj55pjyV1pi+i1iYlYaCLPX5ykdmIa8J
                FdaFzPvl+CRsZGVW5dCXi0MoZ0HX/TgQq+H1rz4AK2flo2aJzQwJtyak44+Ex4SIQ3CoPRIj3T5E
                8p6P9GogxxgECSk/KzV1PFZ/vliXhsZ2w8Yw2cblkZwfzuVxMTH4xfBhmJ2ag8pQJtpdcWgl2Kq9
                AdRbwKViJ7gafE76FZcRVN8Zz7Mu3Dz0RVqVRx/UzUL1U4+hdvNy1PY2oYW0aGE006UgRlFNp8Zg
                GGh4H2AIgUIY9e8lwmjWu70WfbjyO/gIViJeD/o7+qGASC+u9HG9RIw6EHRrGzXztNMoeNTqpeZ6
                K0ZjGsFNTUr6U6vY3x1W+nBpF89aX8OxzlZHwgOv39qP1Z+X4Z3HF2LKea/inMJ7cErSbTg95T6c
                mPMQjkmejpP8UzHZNQ1n+u5lvH4bDk+YgtNTH8CF/um4sPDXODvnCZyX+hBuyX8S0wtomY9gWpo6
                Sz6Iu7Ifwu25j+DG3IdxVc6DuKHwUdyW/SBuzrgPl+fei7MmPoxLRj2Ca9IfwfUpj+Hisb/GRQUP
                0e7n/EO4bOKvcMWxT+GyY3+FC494EJML78JJadfjuISrMMp/FmIduRjmTEGkLR62GMr6WC9+5IyB
                e+ShuDbWjg1J2eimlG+mg1uQotWztjVOL1A1yOi0DR6aakMLXvu35r8BVkbNCAgGVjLBwXQJkLpS
                GChYaDuBQnDSvFSM8jBhpdKbEFHpDXy0XupMX8rRqKEK3zQSqfYtsITvW+l1bAKVCfu0XFOZ9q9Q
                VG1qCicVBiq01HFqJFLtX21hSqd8BL5RowoxKjfD+rBpFhXEd7BKTcFvBsGqQqHSENd5KFNlIWDt
                z2p5vHVUS+W8pmVxKfiI+zkxIhKxdvWzSofTk0vVlIH4KMI41g+vww0PgeR22WkajSGC4DoUNtdP
                EOP+N0S7f4wk+7/hwhHD8HlKFvYwemijsmpxU3HLh/xJaPElEVZUWj43fUs+lfBn8/Acw81NX6RV
                6cMlenXt5FNR+vvforOmBK3W2yQsp1JDgpVFqH0LSC9r2Kaefkt5qf+neh0pjZb3qwFnr2yAXYJc
                nzUMQSmz3MUUJbRqzquhp5es6kIb03cx306CoYfT3r3t6G5vRVdLO8NLNS0NHM7fHVY6hb6eNso/
                EqqXMS3j4t7WXnTU9qClrBv127tQXtyKHcvqsOzTrfj4pUX44/Pz8cojs/DClC/x7M2f4eHrPsad
                l76Fuyf9Fg+OeQqP2+/CDfF34+rkKbglYxruL3wQ9+VMxX0Zd+ABvcKSPxUP5d6D+7PvxgPpd+Kx
                lNvwZMadeD75VjyXeBMeS7sRU0ffhsezb8fzSbfjudAdmJY3FWenXIcTkq7EcalX4EiGfqNDk5Hp
                PR5JjsMRso1CQizDQ3sBEh1ZdKiA9dXcGD290ae3Ak78OGjjukjc4fJiW2Ie+l2JaGWoVq/CQGtg
                7drgi7eskbCRNXmolAi1Rk7Nuv1Zs5cF5wBhZUJAgUU2lLqSKRQUrGRqB1IYZ0I/mVFMSqup8tUy
                AyuBwsBHpnXHHHOMFfppOGMNmieIGLApH+UvhaR965h0nDJta9SX5nWsgpX2K6Wm0O+yyy6z2sDU
                k11plYeBm7YRLAsZAhpYZRBWeYTVBYJVKAkb9sGqbB+shrrOQ1k9rY6h4P5M6rfL6+P9Z/gfSsM3
                gUScGxNpjWllj/HAFhuHSIcfwWgXAvYAfPYgPDYf17vhtrmt13GCHh/iqM4CXv52O5EaG4NJDCXf
                TcrAtvgMNBM0UlIGVs2ClSfAeQ+BRZ8ipL4zAkrA+s4IKgGrmr63ndsU83psv+BSNMz4E+nDstpN
                ZaEHW6SEoGP9Vp9IjUG3r6uR9ZoNTR3D9dpNOyHTupehZF8N6qmWWggy4U0I6+NcP2G015Jqige5
                AbcV+uo4o3EZNDhLOzPr7uqhCmPabhqlmCJEHcN/CawGJKGMZ9/XxVCW1iVw8RCETBlXddS3onrX
                HpRtL8GOjSXYVFSCtSt2YuH8TVg9exPmP/onfHbsVMzxXIpPQufjo+Rz8GHKWfggdRLeSTqRYd9J
                +DjjVMzIOgMfJ5+KT5JPw2dJp+HLhFMwM/U0fJtwEr4NHYevko/Fx3knYm7myViadArmM2T8Y87J
                mBQ4Avm+w1AQPwE58eMQ8mTAZYuDjQ4Uq2GLnXQsSvIQHSkl0gUva0GvPg7hjoGL0Brmj0GuKwb3
                USntCOVgL2HVplpNkLLaEGiElwUpWjNleBMdplkKjFMtOxBr+StglZ09oFbCoaUCHQ4smfltVJRC
                MqU30BAMNK9lAoEJA7WNloeDSqZtZNrvEUcc8V3egpRgIjOg1DEpL3OMRk0pH22nfPRb+9E6AUp5
                Kg8dl6CnfLTO5JlDMBXy/DX4XiZhlU5Y5e6D1Uv7YFXpJKx4Pyp0X3g9D6a18n72EEBdVD9dwTQs
                jkvA5e4IgukncDnUwTgGtvgohPzDCSQH/FSsbocLTpvDGpXWa/fD76KidCUTZIlwR4cQivTimOhI
                KsNUFMWlWQpJ7VU1BNOfYUXVHfBZsGr2JHxnTQJbmDXyuGR1rCRLgz6UMzrYkTkGJbffCVTvRl9r
                PaGhTt9qR+JfB/9tZ5imckvIsFSLNabooo3Kq6W/2xp7rpXxm9XmxZXq6W7FcxonqruK+ZQwn+3c
                YCunW9DbV0pINaGV+9GnGBR6SthJyPVbnU+1v17spZL7L4AV/6QTZWqEtxriDaEGDkaxai/1pmJg
                QMNLtHL5QA8OikE0ajycxnrU/+5drDnyfBbQo1ATPx7VoVEoicvCBt60df4gNiclYntaKlVNsvXV
                4l3BVJSwBtoWTMeWxAzOp6I0LhE7E0LYlJGIKjp3Y2I6SkMpmJMZwgn69JYtFoleDxJ4A136IIQj
                AtFOOpcvCh4qJx8t5HIgexgVgd2HIB1Mr0z4nVRXhNYRbgee4L7V0A+CpJ2FocEvYLF2k3FZ0z5r
                5nEPwGqf8feBWOsBw0ptS39+1cbAQIU8HFgy/dZyzQsAUkHqcmDWmSeCykOwMA3iAo3Jx4BP81qm
                9NpO7VeaCj6DoanpUPkJTAZu+q20ylvLBCftQ+s0L9N6LTOQU34FDANzs/fBahRhlZuN8/fBan0g
                BRWEVSnvT7kqE1YcB2JSKOb+/SVrcyeg155MWCWhy5+OpfS7a+OiEe/6N7g9w+CIi4AtZQTiQj+F
                NzACTm8k7G6GiU6FijbY7QQXlZfTHg+HLR6xUfHwRvox2haDR6jSFvP4q1gR1lPFqw208TtYEUC8
                7o1UgAcCqwb6Um1SAG3OIPbYQ9h81Ilo/+IjtNRsRUN3HZoZEVldKNt7KKz05SoWYekP/jfwFksX
                Q7lOtFGEtBNMA0qKfxp9oaeG0mob0LIUffUzyKrfo7v8efSWPom9ZY8B5Y+ht/p1ipTZ6K1bj70N
                BBo3UyO8mqsEQe3Dejm6o/2/CFaGTeKUkGxNdLoDDXfW6A2KV0lgK/6lUQmimWkbhTIFzps2onLK
                /VgVykdlZArao1lzUcZ3MByrJSxq/S40JQcsq/E5UWO3ExRx6AgmU+omojyQgDpJea/We1Ee8qEz
                ng4VTER1IIivkmJxeOS/InrEv8Bu+wnsrmGIsg9DhH0EopzRrAldVE8+OCnN3bFBxEWkM/xLo4Ws
                R8w+h4fKy47TPF68zH2Vx6UAAT/aA5TbeipjTE9n6GRNcqBBThTuXH/J/hpYCTqCgOyHgGXAohDQ
                wEHQMCrHqBXBQMsFMQFIQFD+ytPkp6nSaGryVjptq/x0HEZ9GeVkABgOK7Nfk0bHrn0ZqGm9jkHz
                Sq/1SqtttG+lzc/VE0GCMTcfaYRVjmCVloIXCatiFvZyFtY9hFUZ79d/aM/5C9bI6//dvfsL1uZL
                QV8wB12BdLQFMrEkPgU3JHgQ5xxGEMVY3RaGa2hjNbjbfobhtp9iZOwhDA3pd075XASiXFGIplK3
                6aO59GmP14kU+vVtBNLX/mSCNhE1bg/DzuBAVxivYBVCbdxABRl+PIJsoyC1zxQGNghWglpqPI8x
                HjX06y2hLCy75ELU7lyK6r4K1HTXszzuRWOnXoNTI/gApnqporoJka6eDuuVOc3rNRyVafSrG9Iy
                ao4PgapH0bLjIlQVj0b5mjhUrHagehXL6yovGlYyUlgxGh1rzgG23Mu0n1Nt7SQbuqzBWiRbGIOR
                HURXZ9d/EaykFwUhTQUrmiYaObCHEk+f8Oro66Rs3Gu9P6jzbSafanmMLdKc7S0oe+4FrD36BKyN
                9aKCIVlTVADdrLW6Kecb3QGCyIemEC88a2Q1ZAsMrZTf7b5MtNizUedmjO9KQ6sjGU2OBNR4EtHn
                zUK/Jxdl7jS853fjCEc0pXgMPG6n9WQm1ulDTGwAMbYQ7LGpcMVmwROTjeiodPwoIogfcT//6vdh
                BMNDF+W7PzoaZzvceI0A1eB47Y5I1nCEaSAZdcZYA9arLxUdyzIeh7FG/j4QayGoDghWVBYq3AYq
                BlhG2ahQy1TwZVpu0uq3lIymSqvlAorWCVYmncnfQFBTLTcAE0gEEZOHzOzHHI/Sh4NUac0y7V+w
                UlrBScdg0smUxuQrUJltU1KSCapUa8A9A6vsPHVdSMELCUlYFxyAVQlV1R7CqoYF+kCs3st7xXuw
                P6sjHGqceiLHioh+uCwxE7cnJyEQEY3oQzyIGplKxZ6PuNhC+GLz4aRf2SJTuTwBUayIYxxxrCBp
                rEij6ZvW6zdOGytGJy5nvh/Th3YFklBNZVUf0BNiwsoj/0hEbXwifY1qnhWjMT0NDDc9DbSeCBJW
                NYlxKGM0UUuAlXpT8HlCCnZ/8Uc0tu5gfEOpQ8XU2K+m82aKihp07G1kmNfBkJClWBLIepmZUOsq
                Rnfz12itfgkdu05E05axqCnOQsUaVt6rAqgirOrWJqOxKBMNa/JQt7IA9Us8aF4yEj2rWLlvPRuo
                eRzd3XMYGFaigdBQC5cldNr/i7ouWHDS+XCneiT5HbAETT1VUOcy9ccwQKO1keZNXCBqY9d2bPjF
                5VhBJ9sUG8UCOpIOEUBXXCq6QsmUwqxdXE5KYR+aGRI0BgkthnaNDPsaAmm8WZlUOmmootqpISwq
                eLNKeDObGao1xeWgiOHik/F25FNix8boaYwPPtZMPta+HgLBFZsIRyQV1MhE+EemwDk8Dj89JAI/
                8znoTA4kU3WNptMc7nTjlmASPuRxbfcEKdNjLNVWJXVHq9GUDqZjqJXR4WrlXLS6AywEsuYDhFWe
                noaxEIeHWbJwKMgEFQMNrdcypTHp1N/KAErrNTVw0Tb6bdJqufZn8tO88te80hg4mrxMOrOtTPvX
                epmWG+Wk3+ZcBC6Tl9LIBFftw9onl+fqY685VJGEVero78OqbB+s5AsKqQ7EagkF677tx2oIiGq1
                JVHlNMcnYWVSAu5JdSN46I8Q/X/+BZE//hmih0ci8LNIuH5C9fTjaET+awRG0CJ+Eo2YkU5WkAwD
                CZRYTxwVVgAjnV5Cy4OzWWG/TX/ZRl+rchNW9Pc/wyoZtfT9gT5XPF5jhGZNuNF/qmmVhNVulpmd
                zKfKSeXDinyezYf1U29HY/ECFsZWRjwDr+AIVq2oQmt/Azq7O6yHZXo9Dl1qh5qL/trn0FJyHao3
                H4+yYhfKi+NQvaEQjZtPR8fOa9Ffdj9Q+QwV1IucvoDesmcppC5G86ZstBcloXNNFrq2nImO2se5
                r+XW3gQrixnt/1XKimTSE4ReyshedWHYByuprb4uyko9ShA9rca4LqorxsFcoG9sdDeT2HPmYf2R
                p6GIN2+HIxYbE6JQFgqiPpFKJZ4ynmFgNWFVx/CulgW3klaTyJqTN60ijmkIqd2s3fckJmBPfDxr
                pAA2B0n6+DTmk4VvGRbcHh+JDFsUbDGx0HcA/aytgnHx8HsV9tnhHBmJ+OhYZLn9OMwfj1O9Xkwq
                SMUlY7Nx85hCTB81HlMLD8NbE07A6sOPR2X+aFQVpKCSNbyOodKYHEw1oozAqqaTyWpodXTyA7G/
                ClYsuKaAGxip4Bu4GFgZCJllJr1gIFAZ2BjTcpnmtU7byJS3QKLlylP71not03qj1gbvS/PmOHU8
                Jl9NBSut17wBmvLTVOuUp9ab7cw55lmDDw7AKuUvwGp3MAEVhMuBWDUVuyqb/ZnubZXA5g+wEvVg
                fboLL0yw4aTMH+GE7H/FqeMOxenHjMB54/8dZ477CU4fdwiXjcCJY6IxITcGeSnRSAxEw+OMYdjo
                RKzdi0inHz/1eHFyjBtvUOVtYeVXScg0GFi5BStWyKE0VMfJx3i8xnieVeFGSAlUZfTlrTz/3a44
                VMYSVrZ4bLAnYOVxJ6Pm/T8AHTVExkDzTAehpU+4tLF09uiJYU81IVIENHyEvvLp6Ng+CU0bM1Gz
                zo+yDeNQs3UyWktuQ3/NbxkifctQbgu3qWRZr6Nx255SdHXOQXPD42jdfj7aqLQ6Vh2Oru3XkIGf
                cE/V1vNDgbK9e39tVgJK2J/FGLWuDVo+sIIm2FjAsZZaf9ZiKJYd+HKzvprx3eMEhn39nX3kE38o
                /BNBGZ92tpKozEfvRLeWVKL6V7/HttwzsCMmE7udCVibEkQxa4NNBMpmr59qy8mLHiQI1AYRxCbW
                QNvik7E5MY21RipvICHFtKX+OOzxBLCDKmgDb3IFb+xubyZmsZa6xj0CWS4bXG471VQsvF46ins4
                3PZ/gzfmX5Bs/z84MiUK5x+eiIdPGofFp5+BpVdegqLrrsKWa2/Atmtuxeorb0b59Xej96pbgfMu
                RNd5p6J27GhUpGaiKiUTFUnpFkDLCawKwko9pyvl1PvMAhcLgwUxTfWbJqf/DmpcpyeKCjNKWJPu
                IKx2sUAU0+HeSchCYSaVStaYfbD683DEBj4qxCrQBhQq4OGFXFOjogQCNYwrDFMapTfbCBAys204
                vAx8FJapoTwchEqv49AxaV770DqlkYLT8Wp/MnOMZv9mv+apoIGilpnzVBqlz8uT4qNCzOGx5RUw
                DBxnweqc9BQ8nzgAqz1ugiowAKsyFmDLeG0toxoZsMTvrJSwKue0kmbdL6vSkVpOtn5reQW3qSAc
                1EZaQl/c5bWhKjQCJUdGY+lNQbz9oB9z3sjD6q8nYv2SidgxOw+bvknBpvkZ2LxwFNbMORwfvJqN
                e291Y/JJP0VO6o8Q7/tXeB2HMDSMxr+6fTgq0oGXeewbWfmV04/VVaJBypyVV70vhZWwKmkdRzwt
                xHIx0D3DMp6jrJygkpXyeLex0i53JaI8JoBSWrk3HSvjcrHn/ofRtWcbFQ4LZude6+kedRQ0ukI/
                KtDbPg8dVc+jceuVqCuawPAuEe0bHOjenoH28j+gt3Y2C/Aqqq/dLO/qPKpCPqBL1Hiuxnh1V2hE
                CzprP0H3ukvQu/xY9K67EL0Vb6C7v5Sg6maaXkLyP4LKgpUyskAicFhjRAzEZQMfi+9Hezt3oT4X
                SmjBhaZjaOOyRqqiJh6GjkRJmI+eeGqYKw2oYDqKDeRI02/O6PuJXWKY4l9u28fz0gOAdq6oWl+E
                jSech+K0QmwkdLa6g9geSsROXuRddCopi12UyiWcV2NpiX5TdeymY+2i85Rwqr40u+0h7Bzux5aR
                fqyhrJ6fwHnWTjvtGfjGkY2LnR4kRLgRG+WFJ9YNV9QIeA75P8iN/jecl+rBsydOwNeXnIviq3+J
                0utvRMMNt6LqiltQcvld2HLNNKy5dSqW/vIqVF99DXDl1cCFvwAuuQI9F1+ItksuQOfFF6Du5JOx
                LZ/nkZCGzQwxdzFE3eNLZQEgxDjd4Y4bUEr7zmmnM4BtNoLX5sGOWB9KnEE6EgsL1VWFO5HnlIA1
                vCbrPKlYGMjGy0m5OCo5AWMzCZy8QuQUjPoOBCrUBg4qzAY2BgwCi4GPafuRCQymD5PgoanSmHm1
                R2m9tjF5aR+CopYJdvqtfEz+MjXomy4HykvHaI5T6c3xab1pODfHq6m2l5knhuHHon1p/3oSmpqb
                h7ScMRiddQRyCfCfZ2TimeR0rI3PxAYW6q2sPPTCcanfx4rMixKXB3ucPkIgnmGfQJTBwp1Jv8rA
                NlfqwEdJqEJUSVQSeI2hLIZcmdw2mYrEhyKvA8WJDmw+3Ibtlw/Hzul2VDzvQdO7QXTPTkDnsgQ0
                raOyWU9lv84DbExE91IvuhY70LvKjr5iGxpXjsSe+VHY8IUPS9/Lxav3JOGKow9Buuf/4kfRBPX/
                F4UHo31YYQE3iC2MODa4QowWUrGLqmqLzYGtTu7fG4udPheVNyFkASppwDyEr56EOvQEkOrKno71
                sQ6sc43AVi/ByihlOc9749V3o27BKquId5ADXSyUvV2l6G3+Gj0V96J90wloWsGKc1Esp6nopZJC
                zVQmfhvNPXvQ2ldLILWwzIscYsgALmQq6oJVEwt8MxoItd3o3347mtfmoHrtWNTu+A06Oqqth2+9
                e2mtfd+HlR4/CiJWzvpUMgmjJ3fSRhr6oaOnG3st6tB6eQBqb9KJWCMiMLal6VMSOjhtt5f6UW9q
                6ymB3huU6bd5cqBTsJ4K6rf10hCz5dkoTm1qa8b2eV9hceZ4rKYa2SDFZHNjpyOA3VIWLKj7s52s
                PbZ5A7ypLOARXBZBmUtYLUmKwzaGh5t9WXjPlY7DoyMQ63Ti0Bg7bMNHInNkBM6j4z88diw+mjQJ
                q6++GptvugkbrrsB8y65GK+cciyeGX04Hsg+DLdnj8d1hNAtqSG8lZ+FonFjUTN+ItqOPQX1p5+O
                pvPPRe9ll6LrwgtRf/KpKDtsInZmFWBLHB2L0NlF2JT607CNjreNx7adtkNGeO10sXZm4ZBML7Ea
                QAlfOlwZz2cXAbySamuNJx3z/Ll4gbA6JjEO4zPSLVhlF475DlYGLgZMxgwUVMhV+JVm3LhxFqi0
                TOAQBJRG8wY2RqGZ9ZrKlGd4vyel09eYjQrStspbkFGe+q10WifTOuWjdcrXmH7r2JVeCkodTrVM
                +Zj9ap3SKL217xz+zuMx5hQQ4OOoOHMwiWHgk1RWGqd/LRXJRqqPLQzTtnojWfk5WLn5UMqKotyT
                Qku3QFXqz8BObyo2uRKw3pWGTe40/qaSUijFtLt53zYHvCjOdmHHJDdqb/Gh4xkPej6IQfcsP/q+
                TQTmpqL/S1ZKH6Vgzu/j8OqzHjz5gBu/vTcJL9wVjxenBPD7h3344pUAVnxMv/02DXUr8tC6dhzK
                ZmVhxe9ceHWaDT8/JwsneP4Vd0X8DJ85vNgRSMUeVdyMLNT2VJFA+DpU6dFneG4lAVbY/hSeVzr2
                ELglPKfdbkKNSmongbXLSaXlJqw8bhT7Y5hfLMpZzta6U1A8+VyUv/USC+IyNNd/ibbqJ9FVehXa
                t52O5uICtKxNQds6tTOdgN7yO9HX+B76OoqowOrQ2adhz8WCgTI+UPYHGDBQ4hlrsbxTU1EzMdBr
                awV2TmW+qahen4Wmst+gu6tJZKBeIhBItu/BqoXgkQpimgEEasrQT2jp4A8Bq5srdBjqY9G+twPN
                e9vQSJnXtLeZv1sIm2bsbavD3tpy9O7Zhb6SbejdtQU9Ozehc/sGtG1eRytC+5Z16Ny2Ht07NnH9
                ZvRVlPCE69HT0oq+5hY07NyCTX94DQuDGSiiQ2y1e7Ar2kXzYRcL7i4W3P3Zdt6wzSHeRHWii6VE
                jgphG2G3JhTAdt7YVXEZ+DUdLhTxY0Q6R8DttWNcMIBrM/Pw5nFnYMUFV2HH1bdh41U34bNzzsfz
                xx+H68fk4PA0J8YwbMyNtiE9Kgbp0SNw5LB/xf3OaHwVzxBVISiBtKtgDComHo3WM85Ez/kXMDQ8
                H62TJqP6qOOwLSOPhSUF2/2pKIlLp6MkEqyE6z5g7WTtXcLwooxhRhnTlXJeAN5DWO0h5HYQVsup
                HFfRAef6cxje5ODoxCDGWbAq+KtgZUIrfUH5aoJZPdj1Ckv4NkonIMgMrLRe6zQ184KegHH00Udb
                r9jonT71ONdgeToOk48BnACl5ZqG78+Y0iud1glSeh9QdsYZZ1jHqXUmPwM3vcicnUe1l0+lR2CN
                4b3IzUrDJFYoT9AfVvLarvH4WHH5sdnrwxafF7sYLu1hpajrWyLlygJdwspgN+/LDvmR3vUjxLZQ
                YW11JQ1UKiEPdubbUXJyLCqvdaDt137s/Vhwon0bwN6v/Oh514/mX/uw/U4P3v+FHTecEYNjjnag
                cGw8JubnIJ/HmT8qDhOPiMPkU4O4/pIAfnW7H+8/GcDyd1hBzQ6hdSnV2IIEfP1mEl689Kd4dcJw
                zEh3U10NvMheQcjWBv2oT1DzgnyFEPNJuafTZzJZuWdhpyeTlWA6/SsZW1nZb/FQubMi1+tAG+nz
                m7h9iV4PsydiC31r3QT678NnAVumo2nbFWjZNBrtVIOta/xoWZOOjo3HoG8PI4kmAq1nMeFUhQZq
                mDryQ32lLHYoMlP4pxebNYIfebF3bxeFCdWW1ROAJFFY1UpY7bgDjcUJqNuSjZ7G19Hf225FXRq1
                VHl9H1aEUpd2IFWl8E47lYjiP/pMoYbfUht9C+daycQO/tvcUYmG6h1oKtmI7p3rga3r0L96CVrn
                zETl+++g5c030fLa62h4+beofuFFlD/7a5Q/82tUPPscqp9/EfUvv4LmV3+H9g//hO4Fi9C+bCW6
                lq5A3SefYMvdd2ONk9AhrErcfspWWixDJIZw21lQ92fbGDJuz0xGVXIm6l0pqI7mtgypNrA23RyM
                x9xAAu73ehCw/Rgptp9hcnoSnjj1NMy74nqGd3di1zXc/yU34A8nTMIv6AijRh4Kx7D/gx+7f4xD
                bIcg0m6D0+1EAiX3ya4IPEcILk1MwW5K9JI4OrUvGdsSMlFWMA51x52I1rPOQscFBBanFUcdg+3Z
                BdiWlIVdiQw1kliDx9GReNxbCNDtgpVApTaF+HSU0gl3MwzcxQK0i1J+G0OSJZT3Kzxp+Iaw+jVh
                dVRSEGNZUAtyqWQKBr7vdyCw0nqBSi80//73v7dAoBeEVfDD08kMrAQXrTNKSnmb/JWfRlx49tln
                ceONN+K3v/2tlbeORTDT9spb6k3LDKgEHO1DeZj9GhgpTzOgn/J86qmnLBBKXUm5qZ+Y3l0UeNUu
                llMQQgotNS8NY7NzkJedgslp8XiasNJXrdd5BSq9cJyOnYHxKAmOZWHNxk43/cbhwlZHLMM9B3YG
                CaSQH9t5b7d5XFabqD6KuyYlCVuOD6D6ZoZwr8QAs+ygxEXf7CS0fpiKuheSUDrViR0XRWDNhJ/i
                k/j/ixujfoSsYT/C8IgROCQ2Ge7osfipLYAfO+w4xOHEz0ZG4pCf/Atcw3+EMaEf4YqTfoRX7hmJ
                xW+xwl2YiJ51UWiaw0jhN6lY88tEfJ0ZjWUeKiJWLHonsT5EWFE57qHf7XbKqLQI1x30/W0ErD7M
                uonnvZHh4QZfLDYF7AyDQ9gQR3UYVEXISp1h4R5bEEVpNmy63If+uUcSUOloWRWP9pWp6F53FEO2
                q4DKZ4GWmYTDVosK6htVQ6ulfccOC1qEUT9h1N+8b0pwacBORWfdnOrLMU070LP5MtSsjUPd9nFA
                1ydMRzlE5dSlER2IpO/BilmhnRn1qaFJgaV2yjwl4rq45w7+2019RT2Ftn7GlE2l6C5ehdZPZ6D2
                2ZdQes8DKLl9KrbddhfW33orVt10A3ZceytKrrkNu6++lXYLdl31Z9v5y5st03zJ1bej5LopWH/T
                VGy6dTo2Xnot1h92PDaNdFNJ+LGbNds2b5COlcICTfr7WDvsxzYGgigOqdZMQBXlcE0wDRWEyU46
                7Hq3F1/R8R6nk5+S6MXvjjwW808/D8svuBLbb5uGVbfcjdcphS/PzEJ+ZASCBFXUiJ/gJ5H/jv/r
                OIROFoVIfS6JeSTZ7TjHZsOblN8bGSLU2BhKRNOpYuKpgBKwiXBZm5CKDaNHY8/JJ6LpgnMZFl6A
                2mNPsIC1MZSKsrQc7E5kzcfafTOPfTOBtZWFYrtfbXRJlu2gstrBc99OWOtjrgsJq6UMI2exkD2b
                lIOJiQGMSU9Dfm4+YfXnNqv9wUqhlYYevuWWW/Dhhx9aIxto3HSBRe1OBhoCjMBhQjqBQusFCOWt
                NDLtV+/xSV1FRUXh4YcfxpVXXmkdh8Ck7TQvqJi8ZcpDv82x6beOW2m1TykpKTQdq0AoeEkB6jiV
                RhDUcamfVXauYEXVl5eJcVn6lHwGzspIxrOJyVjnT8dGhoJStdvcOdgQPQbrozI5DWCzLRbbnJG8
                xhGEk91SIJsYOm1gui3+kdiWGYEdJwSw84Y0VL+SiK5vfEARQbUqFl1/cKH0Li+KzvRj8aggVqS5
                sYOheTnv7/p4qvhACMdQkbudsYhxxsHz0zgEfZHwxx8Cp2eENYSxj+FZkCGn1+ZC1M/+L0Le/w/H
                H+XGI3clo+SrWFR/60TPsiRgThoanvGh6LSfYlnGIVgbtGNrfCKK1ewRTEQNFZMeCKgJQVHJRpsd
                xbHRKHZEMuwbybIxnGqK5+j3Yb0eVNHXdjHErbZnMBTUsEPDUHR6JPo+PRJ9G45C1/Y70L3rLaD6
                G4ZtWwmURgs23Z0UMZ2d6OxuRu/eegJDg/VxYoGK8LAeqklRqTe85rmc8LGUV28bQ7w1zPM11K4d
                j7LVKWjceTG5s5Zp+tDR3Y72LkZqfUO0WenFwi7GknvVOi5Q0QSurh69EqMmsQ7uoBG9pVvRtGAu
                yt59ByVPPY/yqY+i+pb7UHX9FOy69jZsu/UObL//Xuz59ZMoe+ElVLz0W1S+/Bpqfvs66l77vWU1
                r76BKi4r57rS519G1SNPo/b2B7B1ygPYfsd0bJl8MTYmF2KrzYtShlbbE+JQHKewLh2bWHA3UYHs
                z9bzxhW5HXTIAKV8ErZqLKPkRGyK481hTbQqJQuzxh+Ft888F1suvgZVv7wV66+8Ae/9/BzcNHoU
                jmVYlcrQ0EPlFO2KxghnFEZwGuPX8LNeuAgUD8PKxAgbLoqy4R1nEFuoBKtjgtht86E8NgGljmSG
                DolYTnW4kEpvzZhRKD/jVPRefhlaJ/8cO8eMx5r4ZB4TjaCyjKGHjn+jakLuY7NkO0ElSG+lA26m
                Y62ns89jCLCYocnXhNUzSdk4grAanU5Y5OQhM//AYSUQCFhqr/r1r39twUAjewpCgpHSGpgINspT
                22iZAKap8ldeph1J2yqtRkZ44IEHrBeatVzHIrgYwJm8ZYNhpWVGVQlEMo3DrtEXzEgO5vjMfpVW
                09xchqz5hCpV5vjM8cjLzGUYmI4nqFRXeDKwmuFQsTvFaova5PBgg82BLVRUqhTLQ+rmwkqDlUKR
                k/fMnU7fK8Su4zyou86Brl97sHemB+3zCI+Z0dj9ug3bpruw5Xw3Nk9wYANDtHVU7cX0gd2ssEod
                VEIMx54iRA53RPPaRsPpCiL0U16HyGikum3I1HnQR7zRftgj/IiK8GLYiFiMUHeaOB/y8n245LhD
                8dr9fqz9KICG+bHonPXvaP/jT1H/EI/hEhfWjHdjQbzD8vmtTqpDvVRP399uKfV4bPMRXDy/LT5G
                Fj4npw4u9xHGLA/uOMKZoa+HipCV4Ep/DFYcm4j2d68Ddj+DvU0L0Ndagd7OJvQwPpOesTQN+WN9
                KlAP5DT4lb4OY4V/IpJarPTSjMZYGGhkF6u+g1XPJqqq5xhmnoHda+JRufF4dFW+wrwUMuqTfHXo
                6KlBN3c0BKzUHsWcvsuM/2uYBn3NtIsUbKxB/9ZitH8yAzUPPYfNN03Dxuvuxu67HkTDEy+g49Xf
                o/H3b6L+w3fQNOtTdK+ch5aiVWgpLkLbhmJ0bNqAri2b0Enr2LwRbRvXo2X9OjQVrUX7ggXonTED
                dbM+R/0H72LPNTdjfSgX6zVMbCqVRCqlN51oY5wG10vicsJoP6ZappgKYbMrgLV0nsUsIAuT47Al
                jXJ51GhUHncqdp1/JXZefy/WX30zZp9/EZ479lhckJSAvNhI+GJY2xFOsYGBL45EuO2IVvcGv4sh
                RwBu7sPNm5wYHYvLnG68S6coZhixg+HqWhdrVYZqpZTgO3ypWEvQLPQFsSQ1DVsmHoGOiy5EJ0PC
                iuOOx7qMLCxj7beazrrOr3McgNcGnQdBJdtEWG0RpAmyDVReRQTjNwTuAn1kwp+FpxKzMCHBj8K0
                FOQSVhn5f+7IuT9YqXALILGxsXjyySetURIEKy3Xem2neeUhAFlhFvMdUDH/sWuBCQsVmmkoYo2R
                df7551sN7VouuAlUMv1WftpWeWsfBljmuMJhpSGQ1W6lMeIVBmoEBo3coG21XwMsHVshzz2L1yAz
                dxzGZUxEdjrVY3IaHmRoPY8VyHxHEIvtQazhfdrij8QG3ueNLh8Lcxp2xOVhc1w21rJyXJvuxdbD
                /ag+MxVNj8Sj+20PMJNh37c/Q+07/4YN9x+KxefFYv7oIJYnsUL1e7GZFdlmVlobohg+UmVvozJb
                EsrEE+kpGOuLQIybfuX3IDjcg7gRfiRExyPFmYT4WMHKA7fNA5fdBxt9ItLmRpTDi6hYH0aOsOOY
                0R5M+0UMvnxqOGpmjgCW8Vhm21n27NhzC0O7sxxYmxeD1cFoqiM7ioJeFFPRbeB5b/KnEVLprOxS
                B9rfCKZd6mtFv9vsieO6eGyhT+1gaLiMEcfiw8eh+uVfo69iI0UKw7i+LvT0dqGNPGjt6bUG6lPn
                AItL+3hhDcHACEyP5ayGdP6SzNEY7dbjN/XRouBBNwPGpvfQVXIudhUxYtqQjKayOxharlNiZirM
                VaFnbzVhNcSLzB16x0c9wLg/HYD6VgiSVpf6Ru5gbRFq//g2yu5+BOUX3cUQ7h6U3E9F9Nb76Fi2
                GCjfzgMopQCr4jZ13FA9KfTCsnkiMGB/fkJgfnNHkoRdtTy5JvTWbEfFo7/CivhsLI4PYGMawZOS
                gNVxhJY7A8Us+EUqsPuxdbwRG/xBq9vDklCyNcrit9mpqJpwGPrOPgdtV1xthadbrpqGF849B+eN
                ykBm1CGI/dcfwR/D0M/jtsYKt7Hg2hx22OyxsDtsBJXNGiTNRvjY/QGE3E5cxcL3bkIyVhCKa1hb
                fRN0YSWdbIsjATsJ2O3J2XSeJCynVC/KzkXtKadasGo552zsPmIiFkY7sczpwxrWauuptDbGUwUy
                fTHhVUxobeA5b6LDbWDtXEynWk0Az6ZjzfMm43M/C0JiJg5L8Fmw0vcC0/8KWMl0ngrZfvOb31hq
                SO0/AonyMGpFpvlwZSXoaGryF4y0ndqsNKa7RvYUqAzMTL4GUgKf8jK/lZfSDQak9qsQUOPD61jN
                4HsKV7Wd0gu4gpb6bI0tGI2c/DHIyjnMglVGeh6OZ77TeM1mOgP4kvd0Fu/pEirnDQl2rKHaWE4V
                tcSRSbWaTRWcgKWjorHl7EPRMvVQ4PfRwHw/euY50fzeodjzyI+w7sJ/wZKxw7Aw4MHi6FSsik7D
                OhsrUns81tFWEYjr6HvrEtKxMDsHL4yj+k2IQLT/pxgZiobNFg13bCqcMRmIGRlE9Egnw0C39SJ9
                IiHjk69F63NcLrgcIfxLdCb+/d9ikR51KG6cEINlj2Wg+cN4gOEhZkYCH3P6phulV7uw7jgHVlPl
                LStwYnGaB4sYTSxmpbfclUZAZ7MSz2cFmEffZMRhVYYEWpBlJhTEplA8lrHiXVhwHLY+9AY6ywgE
                FlH0NlG41KKzq5IhWhU6++pYXltYwvXYjasNM6CvKGgwvQFYiT3W13B6CZ/ODeht/Rb9TTPQvvsW
                VBaxYljjRvnuU9Hd+gGhxy20kZ4CopZ8aLSauL4Hqx4NlNfTjT7GmmZIG7EKbZR2m7ej8fV3sP3G
                Kdh05W0omfYk+j74EtiiN6sJmb5G1O1tZPYtVv8JHa7knzo/hP/pQxOWKdxUJ9N9f539Xajtb0IZ
                t62u3Yn1d92Lr37qwUxHLAHgsb4YsoKhz/aYTKxjjbCGBXh/tpoFf3VciKFjFooLD0Pxccdi95mn
                AZdcBlxzDYouvQgvnngCpuVNQF7Ig1jG8tGxwzFs2M8QMSLC+hpziPmEKKEDlNQJlOrJVC+h+FiG
                gXaqLQ+ivW7Eu2y4PjkJHyZR7jN8KI7njc5IwCKvHysYJharvYmw2pKQgSKGo6upmraNGoPGMyah
                +9JL0Hz2WVifyFAjgc4e0hAgyVhDBbVaDbl0pCI6vPLYEEhhOJJoPXZfSXn/dTAOc71JmOHPwOOC
                VciLgrRk5BCG6XkHDisVdgOFp59+2oKVVIqWqZOoYKL0ApPykspRWv1WGBYeCiovbaPB8ebNm4d3
                330X7733Hq666iprW6VVXgZ0go3yUp7hx2mAJTP5anwswVQD8GmqoZPVsC7FpTRqv9KDAUuF6fuJ
                eiqaOwbjsg+jwsrGidkJmJ7mx1eJXsxMjMGcpGjMS7QzdPJgbsCHWfSxL+NdmJlpw7dHD0fx1VRP
                z/0U+Jq24WfYOysKlc9FoOiyYZibNYwKjQqGYds2ezL28B5Ila1xpmCZLRnLYlOwiGHmUoaby32c
                D6XjrZwsnOI8BA7bj3Bo/KH4mW84bPFBuENJcBGyDp8XMfZoREWOgC1iBCFlR8juRlyUA44YOw5h
                ZeTwMeQdHodjhwXxcHI65p6ciKpb/MCLDmAGgbqYwPosEV2/S0Pt43GMfGKx6LThmDX6EHyZNAxf
                M7ybz/NczopQQ82sD9K3XIQUy1RRMAWr1J4b8GK1w4dl6eOx9vpH0La1Ff1696WL/+gVOH1lvZ/i
                pb+WrKhGV38N2vfWq+smrPcJ1R2B+FLpHuhPSZ3VVYHulsXorH0d7VRQbbsuQF3xUahYPQ6V2yaj
                peFF9PZs/k4oDdBRmsyi3/dhZb3h3EfAcAPBTbvsZNrG7SUo+WAGttx+P8ovvgk1T7+IhuKV6Ksu
                YcJG5q+R/vS2tCClDqQDB2m9E6j5fSZ09fJIws3oq719hBdjYXIUTTt3YNc107DsX1jYqSI2So57
                g1hpD2F3bB4vbDJWC0b7sSLezK1jxqJ6/LGom3QuGgiptl8QVOdchA0nn4hfjcnCsQlOjPFSjhM+
                dqfNUlAxDhdiCQONquCgPLbrc992O9wEZ4DhQoJrOJzRwxHL326qqmynHdNT0jErnjLbRfXHMHBJ
                XBArkqi0WHMtZ6go2GwNpGKDJ3HgU1pcvvuII9FERdd58YVoOOp4lI8+HFvSc7GaoFrMMGAhQ4Al
                sV6sdAUtYOlrLEWElT6/tJzHp6/+fhMGq/EJhFVqMrKzcqye2wcKKwMDTRVmaTvByoBMUJFq0TKB
                RWpIQBBwBB+zvfLXb83r6aL5XqAGytMnvJSftjf7NoBTfiZPrdfycGDpt0zHpXz1tFJtVzo3pdFx
                KU+TNoP7H5Wl14gIVm4zPm80cken47Sx8Xh4TAALR9PGU8ke7sSqiW4UHePD5tMC2H6+l0rbhfIp
                DjQ+50fnB4lon5nAcIuh4kfDsOm6SBQda8OKbCeWK+SPzcQWWw622zOx1UHF61KnYy9WyTi/MiC/
                pf9SsW1xZmJJ4jhcO9yJrGFRcHJdBBWXjQo82h2BCHsE7B4XPIEgXG4/VZcTMdE2Kio7PK5YuHyx
                sCU4EO2KQGDkoTiRAHsusQAzGc6tSHBh41ER2H7rSOx4PRp1sw5B+8KfomPOSHR84kTr637UPxaP
                8hvjsOXnPLbDbaxMR+DbuJFUW24stQVYqSZjFf1oOaOX9R4HNsTGYF1iCtZceBnai9ZZg+JJvKjN
                3CrMYon6OemT8VZbVTvLfBtXKcxjSoWFMqXrpgBq2YCumrfRVnITmjaPRcM6Oy0dbdsuxt7GV9HX
                sR4dXQ0MLdUYNQC777bn9PuwohayupVzpcJPBXKNbY0oW7AQxQ89haJf3o66h36NrgWLKPfU0UE9
                rwbGTu7ey5PRk0RDpn0nZGat/QqC/MEQ1DKFtwKb9mUdFI9RTx3biouw8+rbsYy1RxHDpu0JLOgM
                6zYSVttcWfuUFQstbbU/ESsJJvU5WsHaYk1SOjZl52PHqPEoP/pYNEw+E52Tz0b7xZeg5ZJL0DTp
                bDSMPwGfZuXj0qAbvthDWBvHwO1ywMVa2eF0WUPBuHys6QjIWLePAHMy/LMTWDY47VFw2UfAFRvN
                ZbHwsnYdGxuFX/PGzlNbGqV/sT2AlYTcKoWFIYYWdL4VhEsRHVe2iutWEzrbR41GNZVdG5VM9zkX
                oPm0yag4+gRsH3M41mcXYE16DlalZGJVcgZWJWVgJW1VShp/s+bmdfmaYJpDQM5IyMETTDsqzomc
                5BALPcO0wlHI3heuCQAGVircMgMMTQ0MzDrBSeGUzKgnwUrrjApSvoKStlPeMrOt8hS0pHqGDx9u
                tSsZ9aXt9LROabWN8gqHlcBq8tXxhpugZBrUlbe6PigfzZtjU/7KIzc9CzmZvAZZh3E6Fpmjk3Dh
                GXH447XxqJ6WiLp7nah+wIaKR6NR+Uwk6n4Tgebfs3C/F4G+T6lOvgqBToLG1zKx8d54zLo0GksK
                HVhBFbbSF2AoxZDcnYlNtA0uqmGq/jVuKhKq7bV+L9bxfq/z0X9p693x2OJIxbrAKEwf5sX4Qwgh
                VkDR/jTEegklx0hE2SLgcDEM9MfBQ99TBRltUwVqpz/a4dJDnWAsYu3DERczDMcy7cOJuZjtyMbK
                iABWuW1YOiYKyybbsGHKIdjzwk/QwPPBp1RcX3t5Pj7gT150vOpA2UOR2HjrMKy6YiRWTI7DkqOC
                WDaG5SczRDVoow9HoSgmEksT+JuVaV/RCvQz4rLEC816nZflt1/ChmFbf7/iqFaWXL2c08JCzBLd
                QXoQUGj4Cr1Vv0F3yZ3o3HYh2jcch/Y1o9C2Mgsd236J3vqPCAC1iVHq9FDs9KqDlD5R0WOJnYE+
                W0M8DVRHT60QPaSMGJWipmIryt55G5uvuQPbbpyOtrkLgPp6S67JpIj0VWb1ZpdZX2i2ZNXA+u9o
                ZZ0cj4k7V1yrhwcGVjLrn1Z1kehC66oF2HTDtZjNMGolZfLmUAJ26xG+iyEUVdUaqpM1lMNrqDTW
                xKVidYjyO4WhXm4hth0+EdWnnIaWc89DF0Os3l9cjM4LzkHtOWdgz4nHYUfeOGzy5+F3vnRMpnPF
                skaLclNmMxxRSLI/iyXURlCBeXx+2N36CGoEJkYNwxtUfws8hJTAROWzls68ioBd7g9gqdeHJaxt
                ZfqtBwUb0tKxg6FK6egxqCFU2y7+BdouuhQtF1yCxvMuRN1Z56HyjMnYfeKp2Hr0cVh/+JEoGnc4
                1o0fh/VjR6OYoFt6xGFYVjgec0cdgZfHT8DokAtZSXFUFAQAQ6PMrIGuAjIBQHAIh4AKuFEkMgFF
                abVccDIfaxAUlM6AYKg8Tb4mTzM1wFNaAzml1XpNlY+WGzP7GOp4Na9lys8s17w5bpM+K4sKLbUA
                uWlHIyftJCSkTERKQQKuvZL36NVUYH4mMM8FLIxB/9KR6F7+U3Qu+ze0L/ox2udSkXw+Em1vulB7
                f5BqiyF3YRy+celjuEFWjqxo9mNqL93ICnWDM8Tf8axE1RE1nj6bjkcinThixAgLTlFUTw6q+ME+
                NpSpP18s1ZWPoWGiw4NxrEivZuX8bVIhtnsZQcQGsNzmwoooNxYn+7H8iAA2n+1GzZ3R6H5pOPDx
                z3jOtKXD0bfEho5FHjTOCaL0rRA2P+XDxru82HChHatG/Tt2FDLUzWGIfFgI864+EyieQyFRhs6+
                elotgVKJtp5StPbsREvPVrT2bqZtQGtfMdq7i6hwVmJvzXvoKb0b7duOQ3OxF81ro9G6JoDOtRPQ
                X3wVsPERoHYpUaG3V/Zv34OV2pr0kNGSOoSJ4sH2ZStQ++jzqLn4NvQ8+BKwdr3VPd4K2ySjBKbv
                2b51MpFJUlGjy1vykPbdUMeKgVu4qBFdnW1oIcEaeAzta5dj01XX4YsRLiznTV5MSb3Wn4INoVzM
                9aVhMSG1VOFVImu0zFyGU0eh6cyfo/8XlwPXXQdcfTW6L78cTeecjfLjjsH2cWOwqiAHS9PTsJJx
                +XpfLt4IZeNsOlGsMwrDNTyMk8pqCCcZbHbWaPq0t98bgJO/g7GROM42En+kfF7ijcNaDx06kIB1
                rhSsVTjqZ/jKmngF1dpy2goCeHUgDuuojDYmp2JreiZ2FY5B9WmTUEs4Nf78HLSeeyG6CK7eSy9H
                /2VXYu/lvwQuv4rTq9Bz4UXoPvdc9BNorVddiY5LruK9uRpzmP708bkYk5NOSDGU0qfjqVYUKg0G
                wWDYaKpCLjBpKsgIUDKt03bq2qC2IRNaHkieWq70yk/LjIIy6bSteXFaeZp8hwKWzORnjkfptcxM
                w9PrCze5GoEi5zik5xyDnNFZuO6ibHz9SCH63xwDvJ4GvJ2O3tfT0fVUErqfTkTLowFU3mXDpksP
                wdJj/i/mZ/wbFvkisdpOZRSbi3VetRfyHu/XQijyJ1uN6yvpF0vVYB0IEVoZeDjWgyOiIxFJ9RJB
                UNk1HtogHxvKBmBlZ+VoR7zdjVGsFC8jFGcl5GGLNw1b7PQzB5WdzYdF9izMdmXjS1aenycMx5wx
                P6aC+v+w8/ZD0faimworBfg2D1hMcM9jQLXgx4RYFPqXeNA9ixD/zIO239tQ+qINW36XjK6iS4Hd
                hMue3wBltHJaJVlQ8Tx/P43+ksfRtf0BtG2ZhpZN16BtUxya10ehbs3PULXy31CxYhiqilJRv+3n
                aKt6DL0dc1n2S8iXVqKCAucAbEhYtfUxtOuSBJLa2Yu2z+ag+s5H0XTlPcAfP0V32Q7rfR71yRow
                q/fVELZvPfPrZBzaReuhxOvr41INe6pGOuut5SYCq9HqD1/G7aTm6pYtQPGlV2PmcB9WUqEsdMRj
                XUI+SnKPxcaMo7B74nEoP+EkVFFBNfz8LHSwAPf84jLL2i68kIrkDOw47jhsGDOWiiuVjhKHecGA
                9TntRZTrq1yZeCGQjtOoHGLd0XQcyW3nkE4y2CTVbW4n4lmzeV1uJNpjcBpDwffik7CCzlMsYAUT
                GfensYYdaF+yPvFNab+K2xjTby1fSycuCiWjKDsP6/MKsWXUWOw67AiUH3Usak84GY2nnI6W089E
                25lnoZ3hbPfPz8bes84Bzr4AbZddhs7zL0f1+Vdi1vkX48wJ+RhfwPAnn0AYVYACgsCMSKBCLhMs
                BsPGLFMhVxoDAkFFv8Pz0PzgPAfnJ9O2Wq6vJht4me01LzUkiBkAycKPLRxYBkIGTFqvtGY/mjf7
                UBqBKjM3AfmsoPLzDkNq5mEIJSfhlAIfHj7Sia+OjcY3Y3+MWUf8DAuOiMb6MV5smRCPbUclYsvE
                BBSPpkJOoUphiLbG5sfm6GSUxGRjo5cAUkW0P6PiWUvFv5bRwHIqvyWJBBahtTwuA4+6AjjKYUOk
                i37n8rDycw/pZ4NNsLJbsHJYTwezHW6c6/Tik2AmfS4Vm130M/rVOjdVeyAL6+PyGHlkYakvBQuD
                CViQHMCiPDeWT7Sj6HQbNp4fjT3XRGLvE8PQ9aoTLX8KoembVLQtS0HvMj+650ejZfahqJkzEp2b
                MtC5MQMda5PRUTRgneuSaIn7LAEd60JoL4pH69oAaldEo2GVDy1Fudz2FPTuvAF9Vc8x0vuSHNjI
                st6EgZZu6Zqh4TTYvgcrhWBtPe1obe+0+jaoAa3mw8+x6/YHUHvrA6TwQtR3loNikFGbQraBVqvB
                puUypensHbAu6wljP3oYiJp3Cwc+waMRcloZAbahgSqrqbkMjZ99ij1X3IDN6eNROfoYbBt7JEqP
                VaG9GC0nXYKWsy8klC5C5yWXMNS7FG0XMGw6czL2HH8CNo8/HGvyCrA8M5tOkgy1Fy1hbbbA7cN8
                1jSLXCGGY1l4jKHk0W6X1dnTxtrtQMNAJ7dxeggpKqggHSbdHotzPC58HE8lRefcRGCts/pIpVuw
                WsvfRVZNm2CZat01VIuruf1KKsblTh+Wcbo0EI/ldCq1cxUlp2F9ehY2E2DbCLCdBaOxixDbPXoc
                SseMR8Xow2gTsHHCRGwafSRWjD0Wb0w4GkdlJSAvIwkZGsepMB+Fowot5SILB4wKdzgYNFXBV1il
                NFJQmqrgK53ZTmmVT3h+ssH5yQQRmboRGJBovcnXhIyaV95mH+H5GBjJDIi0TqZ5AzJz7EqndRmZ
                3CbPT2gnUV3mIy17NJJT0nByegjT0nysWOz4xHYoPvXFYJafBZiFfjl9YwUrjpXxDN8TUhmqU7UH
                k6nCE7HZHcJWVnIbeX/XE0T7s2Le/3Vx+vIzK7CERCxNEqjUppqBp70hnEgfsrlsNC/V+YHByiW/
                0+flGTp6aMmxDpxKcL1NGK3w0N+o4vTwZYM/FZv8ARpDO/keK82Vzgwssadhvj0B89xeLIiPxZLU
                SKzKH4Etxw7H+smxWHOZF2tu8WPTfV6UPGpDxdNRKHvZhl3vBNC88hi0bRiPlg2ZaFmfybBuwFrW
                Z6FtI4G0pQDd20ajb8c49O46Fq27bkDH7vvQWyoF9hlQtxZoqaD4UQO8RgDdi2aLHQO9Ag7Evgcr
                DX7X092Jpk6qG2vcln7s/uQzrJ/yAHZPeZBScQlqeyrJxXauUgOYRmHoRn+vjHiS7Vumz+jIrJDS
                GLPs7+zlMXegvbkJTfU1qKsuQ3VlCRo2b0TnwsXomDULrc++gParb0HPpAut4VZafnEpGn95Jdqv
                uhZ7L74W7VQRLeedj+azz0HdGZNQcvSxWJ8/GsuSeEN88VhAdbOIUFjKm7XCRQXjUCgWwjI60lLe
                zIWsdaa54zCWNVwE5XgsayrnAcpxl9sNj9eLZHeQctyFHIcDlwYC+Cw+DcWU/lvo0Op6sN6fgbUM
                GwQmQapYjqzlcuQAIcblK1nLLrd7sYymwrKcoeUK1o4rqLpWUnWtIsBWswCtDsoSsIYOv0oqjee3
                yh2PLxleznTE4SNPCh5NTENhnAPpCQGkZDIUKiQACBRBZTCoTIEPB4JUlZSOliu9poKJQKN0+q20
                BlDKazBcTH4CifLUtsrXQMWAxaSTafvwPAfDyuSlbQ2slE6/lb9Zp/1ovbbPylbPdR+hHYesnAxk
                Evhafg5D48cysvFFMA3fxtBHWKGsiEtn6M7r7vRjMf1kkYtKyJ+ClQm8f6w01hE0RQkMr4I2Kqb4
                IeH0fVMfOVY4/iRCLxErQ7xvVFxqs/oN8z7T67deufE4NPUM7WeDjbBys1LUazhOwipoi8VRUdF4
                hSBdyIp3Hf1ubUIKNsZlYj2hts4diWK3gxZEkTOJoWwyVsYmYoXCRSq6VT4HlhPW39hG4GtHBL7y
                RWNWfDTmJ4/A6twRWHtkDFadxzIzdSIa5z+Mjp1Por3iEdrDaCt/lPYY2sufQGfl0+iteR57618G
                Gl8FWt9Hb992sqCa5b19IMxSy7xpnWfEJhnTYzXGd1rdlw7Evg8r9VTv76PW2YtqZqWRaHZ9/TXW
                Tn8Im268G93vf4K9O7eiv70evWprUjhnHQG3C7O9/R3o62210ljtVF1M09LMGI8nsG0n+lesRddX
                89D89qeoeekPqHjiFZTe8hBKz7ketVfcxpDzRnRcdhX2qpvBtVei7oZLsOPa87DlkjOx66STsOfo
                47Bj/BFYn1uIFSykC1nw5zmonGJ9dLgAVnmkYJJZw6VhQyiDNQwBwVpyVTxDwlAmFqSOxh0s8PkM
                34bboxATYYfTfmBhoPry+AirJDp3fIyTeThxVSiEL7Qv7kewWhuXRCBlWMexklBUn6/iOLW5pWFj
                Qro11W8t13o9HVzH0LGIDm3MavcI+631xcqX4eZ6qrj1DDNmE17fcJ+fx2fhmcw8KqsQCjX2OEGV
                NXa0VUCN/RAAZJo3v5VGaTVvnghqfnA+xkxeZnsBJBxw2l5TrVfoJzNQMZAaDD9tO/g4DZi0Tuk1
                b2Bn5s02Vv4FKcjNS6Gy4roChZ/ZOC+PsErLwufuFMwd5sVCWxwhxPAvxc/74mfYFMQy+tIi3reF
                vDeLgnFYHO/DkpATSxJsrCziLT/an+nrOVt8DM3c+jgF76VCfVYuxc5UvOlPx0WsUONi9TUlfeD0
                wGDlJqw8glWsm5WrE25bDEZHRuBpZxCzParMWDGyst4Qn4mtasdKyCU0CWKez1qfn/7opd+4sYEV
                WpE/ij43jNAaSeWYzMqcsCPMNtsSsHWYAzsjmcbjxLx8+thlZ6FnzVyW3zpr+HEGR0M+PNNTO2oU
                60Gghn5pJY7EKRHB+uoe1/V1cnuyYG+PPjtfTzGj9wWHhtNgGwJWzJ5hmnbSSGMytK1YSUn4EnYQ
                IlXX34/GB19G7R/+hIpv5qB63jw0LlmM1hXL0b5qpWXNy5ahftFCVH07F2Wzvkbpx2+h9N3XsOeN
                l1DywtMofewRVEy9FzW3TEHjNXeg9fJb0XnpLailbb/qdpRPuQ91TzyK7bdei+UnHokNowqwIi8D
                SwvSsSorFetjvShOysDq5ExLKS10BbGQgFrCm7ZCN82XZCmataxtVnO6ks5TrA5vLNwrCIoFrDG/
                Si3EFVRFydEjMNIRDV+MF26GdEM5yWATrNSDOsTjCETGooCQuy4xibV1Koq5b73Tt5JyfFUgjWop
                AcsIoxWU6KvVR0rqK6TOn+mEThrW8NhWEW7qeiHlJ1O/shUE6XICbIXUF5ep35V6vxcxRCli7VlM
                pyxOzMDM+JA10unHgXT8ioXwyMwQ8jMZWmmETBbQrLBCP7jgS4nIFD4ZiGi51pt0Ulam0d3kYfIZ
                nJfyMaGdTPNSaupqYNJpH8rP7EfbKa0Bm6YGVEMda3jDv7bTeh2PptpO22i/2iYzg3BL1RPHJKTm
                pFhDxkzm9EGu+4zqZl60hyrKy+urkWPdWO/x00/U5sj76M9iBZFDyGRhNeGykgVaXWNW6l4xtNuf
                raWi2qjQzBkiKNReGcRaqrYiWyL+GMzEpVTa8TE2xEd5CasD9Duah8fpt/sQS8DZ7bHIiRqJR6jO
                P2douZSw0gOntfSFFZ5MLPGwzHiTsFwPeOIJ4gQffYfHIZXH81jhotJypWNLoIBgyqEKzOd8LjbG
                BnncbnzjjMJH2fGYfdOl2LtjJYnTjC7SqJvRUg+Jpa4L3xlhpOFcNIiCYNXeV4fOvXobRb0m1cVp
                oEd7L/9jvGV1X+oUwDj9Dnz7se/DSo3rRKRRbhZGd+9C16efoeL+X6HkmmnYc8V9KLn5Uey+/0mU
                PvQMKh97HjVPvIS6p15G3ZMvo/pXL6Likeew58GnsftebnPHndh9y83YedON2Hrjjdh2003Yedvt
                KLtnGuoefAStTz6DrudeQsfrf0TTJ1+g/ds5aF8zDxteexKfnnI05hJEC2PcWObxUYkEsOWQWCyl
                fJ7vCWIBJe5C2hIW6uW+AUdaRSisorOsYiFeSkjMp0JZ5RZAkrE4lIpZVDUfpeTi7OhoBCIOQYQr
                Fsn2EHxO75BO8j2jw7i8HgRjWMuNiEZerAM3JKfgM9amRYSTXoVZwrBtKX8v4f6XeOJ5HOrCMADO
                VVRUxlbweLV8GW055zVdykKxmNssIuRkmlc4q3XqR7YiIZmgTsVqAutPBMEMQvpdwvhh/labVSGB
                riF8MwrzkfkXQBUOF021TsAQUNQdQMuVVoAw80oTbuF5KY3SGvAJSspHykfLZUqnqdnWHIOOz0B1
                qOM0eeu3lqtbhYGT9qWptg3PvyCbii2T8MsiuApTqa4ScEp2AqYkJeN9guTrKA/mOJ1YTNWx3npy
                R8i4WclRDa11s9B7pEoyGcYTWLRVbjVWJzNEVJj4l01gU/ea1c443vsAFnt9DPVpMfF4kyr4YlZm
                /pho+CPdcNsPFFasJBm+Bej7sS4fbE470qNH4j63Hx+r6SNIJah+fTw3KUM1hyykfywK+qzXgRb6
                PFbXmoVUlUvceawMD2MoOB5rqR6XEqbqvLw2gRWoKkxWlp84ffhg7BHY8PzvsLei1SJLT18Huq3P
                b9G6O2ld6O4mvNR1ifARLtQ9yYrQNNyvHqbtbYc+Ca8upQKVWoOktjoJIPW7PNC/78OK2XT1d1vD
                mFqD8OkLFi31wPZ1aJxFYL36e5Q9+htUTHkSVbfdj8pb77Os4pZ7UX7zgFXcMrCskuurb38AdXfT
                pj2EmgceR9UTz6Lqpd+i6q13UPP5F2igAmsuXovWbZvQW1YK6EOH3a2kcBM2LfoMn1x8Pr4aHsDy
                CMpzFwuwegQTXnPtHsxyeDFXF58wWsybv4QXewlrl8WU3YvoMGqzWsD5b3gTl1CCL+JNmMPfM1jg
                307NwRkxkfBEHmL1scqwJyLAmzOUkww2NbCrp7E3ygHnsAjk2mJxY0oqPlX7hJs1KW/0fDrAAv62
                QgmFFXScxdz/YLPWWeupwKi0lgRTeNxJWEDIzXPF4dt9No/Hr2ULCeT5cSEsJAQ0gsNblOrv2Tx4
                0xmP++moR2cnYhRhlZObg8yC//jZKgMB2WBYqTe4eoWrx7m2kXIRbMIBoIpCzCwAACAnSURBVHTa
                zuQhGwooMgFE6/QkUCMjKK2Waap04aDaH6QM/AQ+NfyrN7ygqp72augXuJSHtlE67T8rMwvjcsdi
                fO4ojGb4l38YYTY6AcdSed7EMPr3LqqrqCA+ZwU1hxXL8lAOlUka7wmvq3yKimuRW33jBBtVImlY
                5sukr/H6++P3awt4v3XPF7r8mEvlNoeg+JYK6tuYAF5NysJ5VOJ2WxQcUU44qfCH8rPBJlipQg2y
                crJTBca4nUixRWCq148P/HGYy/OYF5+IRQ76PQE5P+i1bJ7fi288bsx2ejA71o859PUFBO8y/2gs
                j8vHt4EIzPJFYm7AwXIUYIXPY+d5v0+fmnH8OWifuwG9DYSQ2pr6OxgKdqCnu408akUXrbuLMCK8
                1Fbdr3ZqdVESjUQl0mgvQdbLZb2kmTViDBerd5NUlaYH+vc9WOn5XqdFTQWg5EZbGw9QQ7bXkoRl
                6Gkrxd6yXehbuQxdn3+C1o8/QO27f0DFm7/Dntdfsazqj2+g4YO30f7pR+j+6nO0rytC544d6Cuv
                ZGyp3q3MmP8zDLXOp53TRu6uXYNp1XFJtx5p9mB78XLMv2sqZtlYq8UV4suUDLyWFo+3s5LwJR10
                NlXWHNYIGnnAMt6w2ZS8X3P5V7yBsjnxdL40Ohnl/SyHDzMonz8gTN7NKsTZLOjxjgjYvC4kRQbg
                IwAHO8hQ5tCTHMLKzRDQcehI5MTEUlml4kM6+nJCZaU3iG/9AXzL3/MJJDnufJ9eOOZyFoS5rAnV
                wTDcvuUxLabiW0hYzSesviWY5hBSs9UewTB3DvOdy3NQnl96PPicxz6Toehr9mj8MdqJ1xhiTA8l
                4Sgqh/z0JKSlM7zKGniHbyhgCQjhUNDAeBogTy8Hn3rqqdY2KvjaxgBDEFB65REOvsFwMdNjjz0W
                t912G/7whz9897l35ScQqh3L9IhXY77yM8dktjfQE6QETs1PmjQJU6dOxeTJk613DY8//ngrP7Vh
                Kb3SanvBbHTmKIzJzMOogkzkT+C5E1ZHpifgurg0vO7Jx+eOTMwMpmNGfAo+pAr/JBiPz0M+zAw5
                MDMuCl8GhuNL/0h8FbBhVpwH3yTs8zP53P6MfjibPjDH48VMqpovAm58SUX+BWHxXEomJiUnIiI2
                ApExDtgPsFOoYOUXrOgXdu5Dfptqj8RUKsP3CMg5gRArZvpILCtLqnENHzSX0JEPzWH0MY8+tnBf
                G9yikB0L4wmouAh85nXic/r0LIZ+iykA1tMfl9ps+JC+Nees04HSHWilgOi0Sqtk0380DVfc199J
                XdOGjs5mtLU3WtbJbXr6NdZCWHo1bGl8q17GbZ1NnOohHlcdgH0PVlqqx4Rq0FKCvVZr2sABWR0R
                tCNKvL2CWGMD+hrq0Vtfh546xqe1NZZpXsu0fm9TI/rb29HfxW3UMVRacV/e+tPE2gNn1LO9X9pQ
                56UguKIStR99grcPOxbvJ+fg82Aq5gdSsYYx+bdUF18HA/jS58MXLLxfsvb6mpJ3Nh13Dh17FgvD
                zIDfWjY3xPmgh2Gak+bm8iC+iEvFYyz8Z/Dm60vKP6LjRLgc1ogA+zMvIRigQosjeOJsVGVRMTie
                N/cW3uyXCabPpYxcDOVG+AgcOmnQmGvAAi467380rZ8ZR8dmofiSx6rfcvA/p/lzPrPj3CwMbnzt
                cuIThgVvEn6/omNex8J6UnYqCrNZ6LPTkZKbZSksAUuQECwMCMJNyzQwnl5gvvPOO61377RMEDDw
                MNsJGAZQWmaAYkCm/SjsE4A0gN8jjzxijTyqcbIEkvD8BChBUVPlbfI1YFKeSqdl2lZDI2sUBw0O
                qDG3XnjhBWsEBvOqjQGmpoJVbi4hmJeL5Hyqw4J0jM1JxVkhhoGsCN70pTLMicdMXreZ9JUZ9JPP
                gn76mI/mpblpLnzO6/4578cXXP5FnJ9TzfN+7Mdm+R1UZlFYER2FNaxMljIc/DyQgkcJlDOpctJi
                Y2CPikIcQzqfmz44hJ8NNhdDSTuPN8LhQKw9CtmuaFxCEL7GSOFrDys4ns+XhNJHBNIX8X76iZfl
                gOWCx/wVz8MyPys5vx1feGNY4RHItPneWCxw2LHc6cdKZwhzI6jUbIxCTvo5xcebQEcLqKMoY1RS
                VWK/b3stZqjxXWOm77O94sYQ21jlX7JqHwsO8G8IWP33/UkS6qlBl1ropOxaKLXWFWPlAw/h3aOP
                x5tUHh+w1viS8v1rKomveKO/jKVjxdChYjyYaffha2eANrDuC4ZHX9kYKjJs/IIg+dTJGoTwmMU0
                s+xJeC0mCddFxSE3won/MzICwyIjrSFS9mvRMYjWS6YxdsRGRMI3fBiyhh+KM2wxuIuq7mU6z5/o
                PJ9TrX0RS+d20oEPpnndmMGa8E8E5Yc859cZej7OEPJG1tgnslAfxkI/ah8wzJM1QSQcMMYMOAQo
                qRWNY2XUj2kINyDQvCw8PwMwwUTzWiYAab8KLS+55BK88sor1gif2p/ApP0ZBWQAqt/KU3krn8HH
                p4caGhProYcewosvvog33ngDN9xwAw4//HBLocl0HOHAzCKwUvNykMQwMIXAGpuRijOojG6mUn3e
                l4TfxvrwJsO9DxnufUHVfUBGPxryngyyzz0OVow2zHbEUumw8iFIXmUldiXBmhcTgejhP8Ohhx4K
                e6QdMazshvSzQRYdZUNstAvDDh0G+6E/waiI4bjO7cEbDFNnOpIZ4lEdukJ4KyEZM6jsZhGCX7My
                +4rHPJPqTfal3UmfdFj2uc2Ozwi+T+PoS85Y/Ikq76MYH96xJ+BPo49F8fTH0LFslaWEGokqfYPh
                v/PvfxSsdCnUsN/Uwcui/lndnUBNFTqXLsHCadPw/qmT8NaoCXg/eyxm54zHt7S52ePwTdZYfJM5
                FnMyx1g2O2M0ZqWPwtdphfgmfTQWZR2G2Vmj8UV2Ab7KKeQ2YzE/ayI+yjgSj6RNwDkpo5FN5ZaW
                MtCWsz9LTmXtn5aORBa05LQUZKYmYlxaIs7KycAdY0bjucMm4J1xR+JP+Ydzf2PwTfaog2ozC0Zj
                Rn4hPs7Ix594ru/k673A4zBt/NGYSJV1WGIqDs/Ox3iNFKphUlhoVYDDlZVAoKl+65wECxV2pTPz
                WqdtlFZpBBrBRCGXYKC0Wq60pjHdAEbQUqinkRY0lIuUlfZnVJRgaPYj07IBNZRr5ak8zDFqKlhp
                OBgpP406qtDyoosustJrPyYPs/8sHptGXkinukwfpd78uRidnYFTkwn19Dy8OGYifps7Bm/wWn6U
                z3skHzoQyx79vfsxlH1RWIj3jsjHjDEF+LJwLD4qHI/nRo3HFfm5GJ+RjFByPHyJvF6phDf9See8
                P8ugfxYkZSM9wLCXSvB0nuv0nAJ8kD8Rc3OOxLzsifii4Ai8P+FozBx9GL7l+c3N4fFmFWLOPtP8
                XB6f1s3LY5nhsX0wcSzeHTcKb/GY3xo9Hh8dfwZWT30I9V/OBsrLGfV0o3ZvO9qtN4r/+/7+R8FK
                TwrUv7WlT5KzD72Md9FJddXTjubVK7Hz/Q9Q/NxLKH78Gax74FGsu/9hFN//CNY/8AjW3fcQL/D9
                WHH3NCy98x4svWOKZavunIbiux7AMi5fMOVuLLznLqy45x6sm3I/lt79ED6+80E8c/t9uOXWe3DH
                nXfj7rv3b3fcfRdumnIXbpl6N+0u3HrP7bhryq145J7b8NvpU/Dxfffh23sfxNKpD2DlPdOweso9
                B9VWTZuOFVOmYdnNd2HJDXdh0a334qs778fzv7wR+QxNc0PJKEzPRkHmwKfdBQGpHYFHTq/CrIIt
                07yWmTYkE5oJSgZyBlxaJzj8EKwMsAw41M6idL/73e+shnatM/uVhW+ndFKCRlmFqzYBSMekEUcV
                +ilcff/99/H444/jpJNOssJA9ZLXtjoPgU3bpackU11RrY3iNRhdgDGFBTj7iKPxyPmX4utpD2Le
                1PuwaNq9WDb1Xqy++wBtytTv3Y+hbAl94+v7bmf+9Ddus+iuafjkzil4+vZbcdvtN+Iq2i/vuA03
                0ufuuGtoPxts9zDt/bfejbuuvxH3XHcdnr7pJnx61z0ouvtBbL/9QWy59T6svH065k+/H8um34tV
                U6fS/+7Bcm4rWzGF5YHL1t57L8vMg9j4MMvPIw9j0cNM+9TjWP3MU1hL1brjvQ+wd/MW0mGgTaml
                txM1fR3o+Ces/vzX1duDps52KzYWrNp4kdoEK12k9lb0V9egb3cpsKsMnRVlaK8pQ0dNOTpqaZw2
                V5Sgbvc2VG7biIptG1C5fRNqt29Bw5YtKN+6ATu3FWHX9jUo316Exh2bUL9tK0q3bkMxb8z8TZux
                bvNmbGHa/drWrdi6cwe2lezClj07sXH3VqzjPos3rcO2DetQsWE9WjZsRtf6rWjashG124oPqtXz
                vOq3bEJdURHqVqxB/doN2LOmGJ//6WNk5uUijYDKLBzF+YHOlgKBaRsysDKm3wKDmVc6QUhpDdwM
                fJROy816rdNyQWgwqJSf5hWmCS4KA9XfyrSDhYd/Aox+K1+zT4FS+xColLeWXccCqnYrfYhC0NLH
                KLQPmfq9KS+lU/6pzDsnKwP5DANzqWYyc3NQyGtyyTkX4M1nXkDVirVoXluMljVr0VS8DrVb6SsH
                YtvWf+9+DGXVm4tQsX4l2jYWo3f9RnQVbUDtqiJsXrMaazauwyrew3W7dmDzrt30p21D+9kg20p/
                3c5t1m0pxrqNa7Gd/tbEZdi6A6Cv9dIPGtauQ5l8cGsxqnYUswxwfpusyPpdu3sTmsq3s+zsRmfd
                HnRWcbp9M/aW78Heqgr00jqrq7CX5bCvp4vlrwN1ba1o22uN5fvf+vc/ClZqlOvt6bbme/oGvinW
                2dODjm6iq7cP/Z096G+j2urmhetjWjXoaTuamvI6uW1TWwtqGupQXVeL6vpa1NbWoa6qFuW8AbsJ
                t921pSivK0djfTVa6+rQXNOAuuoGlFXVo7pG6fdvdTU1aCQ4G+rqUd/YgOrGepTV1WBXZTlKSktR
                WVpOh6hGV1UDmpiujqHswbSKxlrsoZVXV6K6rAKNtQ2o4nl+tWgB/FQSgexMJBFUKdl/VkFSHYKB
                VZBZoI3pt0zgMJARMIyZdYKGgZFZp+2N+jFm8tA6/ZayU7uVjsGEfiZPrTd5ygQnwTL8GM3xKd1R
                Rx1lta0ptFTjvdJrCBvla/KS6Ri0TX5eFkbplSMCPD0jE5lZuTjv/Ivx5u/fQn1VHVpr69FSVYXG
                ykr6gF77OgAb4n4MZbU1laiprUJHQwP66hrRXVmHxpJylO+h/9VUo5qqpZEFrqWlDfX1DUP62feM
                /lzTSl9rqMQe+nBlfSVa6H+9jc3ormtAC4+vij6xu64CZbUVqKRV1HBKq6qrRC19prG1Ee1dbehh
                aNdnlRoiqItlynrwNfAOXmcfyx0FQjvLXxvXtXdSPqiQ/Tf//Y+ClfWwwHosyIumcbE4r4vU3sWL
                19XHa9qH7m71qSevega+vW/126BpJJqOrl40NbfTSRpRSUBVVNIqalBRVoPSsiqUlFegpKIcZXTO
                mqoaNBFQrRUNaCtvQGspHaaSN5vOuz+rYT71u/agdk85aiuqUM28yqqqKfjKsZ3Kb9fuMlRyn001
                zcyT4CS4Dqbtrq/DdgJyN6FZzvOrq29GVUMTZi9dCk9qCoJp6cjIzqeyyLdCKxMKGlgZGJh5Ax4B
                SaZ0gpvAYYBlFI5M640K0joDunDTMoFD2wmYJn+tE2S0TnlpmclDx6hjNesFIO1Dx2LSmXDR5CdV
                JtOx6rfJU8eXk5+FAsJKH47IZliclpaNn593MX771rsob2pBZQP9hNewqlLXteYA7fv3Yyirok9U
                EkKNza0ESisaqutRSX8pLSnlumo0cF0LIdNMkNUKMkP42feMFW41wVNRVUYfLqN/E0A1dWigv9ex
                0q2srkNJdTV2EJIlXFdaVomy8kqU8/yqmK6OPtJMOHZ0dlMM6Kn/QNnpYeHpohjoFKhYtgY6hPcP
                9LW0uqcrkRJzxX/j3/9MWFFBWRdHLe68QLyG6OzsRRdBZYGJi7uprqyv7jCdRhPUtKujF60tHVQ8
                TagWqAioslLeLN60cs7rd6UciQW8hhBpqGTNSli1l9Wjs7QedRXVqORN3p9VE0p12/egZieNtaVx
                zrJyqreSMpTsLkdVWS2aalsIq3pU0ckPplVSEVSxJq2lOqjfU43mygbUcz8LFixBYpIaYjNxWPYY
                TMwZg0IW7PBQ0ADGmH4LDjLNCwJmlAbBQ7/NcgHFAEOm9cpT22q50hhYCRiaCjpqQ9JU65VW2xg1
                pWX6rWNUh0+1PRnwmPUCj7bT1JyDHuVrvQGl9q95rRPc1GUjJTedwMrG6PwCjM4qQHZGHn5+wSV4
                6e13saOpETtYsEsqWPhZoIe6zn+T0f+qeJ9qG5pRU9/EyrMGe6iqSneXWJVdI4HVxP3XyZ8G+dcP
                WVU5/W73blaUJbQy1O2iIiyh4tpDxVXK/ZXSN0rp46UVVHBUVzT5fDX9RGWilRV5Z3s3ell2VGZU
                3tR7oLWzE03tVHldnQSURkQhnwaK3r4yyQnL317r3Zj/vr//WbAyf7pAtN5u0r6dISCnGv/ZeomS
                y1Ur9Oy1Bj21TBdWpvlOho8tirMpsysYmu2hHN9NyVxWV8UCTkgx9KtmrVbBWrWcVsH5KoZztTUE
                AB1oKCcZbDVM18yasp4OWc/arIG1YyNryfr6RktlVRF6dXSQtnrCSvuifD+YVkdl1cJjbquuReue
                SjSXVKCazjt39lwkJiQjNT4Jo5NzMD4tn7Aq/F671WBQGRgIDIKKICRTGikVpdO2Uj2CmKYGVjKj
                wkw+Sq9tlUZplafgo+VKb9ST0pttdYyDTesEIjX8K9TT9uYYzfYyLTf70D4FPW0byiVoCatReYUY
                lzUK+ekFOP/iy/H6hx+horMdpbqWDK/q6RNDXee/xSzFVkF/IZAU9pVWVaC0ohSVFWX0mUpWZNVo
                rKu2wsWqqu/72FBWXVmBJgKrhQBqJZxaWMHW76lH9Z46VBFY1XsYfrKybKTqrygtxR6mr6il8mZZ
                UJlQ2QgvLwOm31zOsLC7Vx07O1nOGCIyBOzrVWdPgo3prJeJOf3v/PsfBStdPMlQkV3gF5S6u3k5
                Sah+SlV9814Xr890KlP1YNk++SVjzbCXF7uztQVNdaxtqssJrN2ori5lTcabzAIu0NSy1iuvqUUp
                JbhCuEo6lqR2NZ1r/8ZwgPmUs0YusxySTk/w1TU0oEbr1JmV0ruFEKun09bQOQ+mVVYplNiDBoYD
                DWWlqKGVlO7GzPnfICE3E6E0qpXMHOvduFGFo/5DfysDE2Om0BvoCA4ChQGKACCYaJ3SKB8tN9tp
                nYGWgZMxk6dM8NJUxyIz6TXVcZjfZpny1XKpJZm2DV+n35qaea3TvPYroOZymjyOxzqa4MvmssRs
                ZMal45yzL8Rr77yHstYm7C7ntdtTgobysiGv899kBEVtGUM1wmkP79Nu+l9J9R76CvdZXcbQj+q7
                huusZQTRkH72H03pSrlNFeFXQx+urmKFW9GIXVTVu6roy5X034pyNJaWEFy7qbD2sNKsRBtVZC9h
                ZX3P6ruywnLD33pFprNfr7fpdZl27LW+XKPhEXr4m9EMp6096mM10A/9v/PvfxSs1NynzvmCli6M
                ZZJS+/5Ed33Cy1pigemHjDl1daG7tRXNjax1almbMdZv4Q23YMUbq0b3SkKqnDVWmUIBhnZyiBrB
                ZZANdpoqWjnBVspaTlZOJ6liDV3NWqxKtSBlfhVNDbK1dJZaOe5BtNKy3dhTtov7KqVzClzlKOH8
                N8sWYtxxRzH8IZRYeAv2gUoFWIXZFHCjSAxsTCGXaZm2EbAEBv02kJApj/BtDSi0rVFE4UBSGs1r
                qnQCiX6bbTRVXkqreZnmw4Go32a78Ly1b7N/hY/mHCzY0tJG5yFrFJdl89wT0xHvCuLiC3+B9/70
                McEh6PPeEFT1hMpQ1/lvNoKjqpKqnvdnDwFVSqugVRFelZVqdyrF7soSC0JD+d1gq2Blu5OwK6Ua
                U0VbwWigjNDaXVNntV+Wcn0lFVwNIdzAPFuoGNsbCSqGeKz1WZhYosLKiAUvVuzd+iqNSp66CglU
                3wkAhoT9vWgjrDr/Cau//19PTw+ampoYpjWigcqnvl6qijE+b64FHkJHEruCwNJv4xjhcDLpfsjC
                pbryCTctG2qbv8VKKfFlJm8dr6arV6+23vFTNwEVchVaU6CNqZCHmwGEzBT2fwTT+UiZGZgJfHpy
                qM/OL1y4ECUlJWhlZSa/MNfvYNpQvmTWDfYTk36obcJN25URrOWsIMO3C9/G5N/e/udxov5R/v7h
                YdXH2qSjo8O6eXLO5uZmy0HrWOvoBuvm6sbLAYwTGAsHTbhTGMc4EDPbH0wzx6rjM4+1dbybNm3C
                xx9/bI2eoH5HMTExVhuPaZ8yashASoXaKBXNDwbbP4LpfHXeMnV70MdRd+zYYfmBrpugJRvqOv+t
                NpQ/hPuQMQMvTY0NlYd+m7ThPqp1OhdVxjov+bkq6X/C6n/Zn26WgNXb20sl3G2BSzdTaksqS85i
                nEBqZc+ePdbU1GDGcYyjGVVmnMmsM+tNGmPGyQ6m6ZgE25aWFqugmePUdOfOnXj22WctYBkwDbah
                CrVM6f+RzLR3SWFqJAn1eF+xYoV133Udt2/fbk3N77+nhftIuH/oPmp9OID2Z7rXZqo8dfyClHxb
                kJKvW80l/4TV/84/Ay3dzM7OTrS1qUNei6WydLPlNCrsg2EV7hTG2TQ1y40Zhwx3RuWpZeFOe7BM
                xy3nFJx2795tLVPNquUqkK+99hquv/56nHDCCdZQLRr7ST2+pS40XIteMtYrMOPHj7fe29O8epv/
                I5nOU9DWdXj++eexbNky677pGume6drpHqniGnx9D4aF+4Yx4yPhFu4jZhsdp/xPfhhuJj9tJ7/V
                sStq6OrqsvzbwMmA6p+w+l/6pxunGseoLEErPEQUYMIdLHw+3LRMjmQsHFxaL+czTvn3gpWOVXCS
                A0tdmUKowqflu3btwqxZszBt2jTrvbK77rrLaq/R+FI333wzbrzxRqsQq41LpnmNYvCPYjq/6dOn
                W+8lzps3z7oe5vrovuj66d7ot+aHusZ/qxmfCDfdp8GmtMZnwrfVunBg6beO3QBWfis/Nioq/O+f
                sPpf+hd+48JNN1mmGkkmtSUnkMmxVXMZpw53PuNA4cCShTugTGmN8x1skwLctm2bBVvtR8AyDq71
                cmidQ/g2Zn24mWPX+SjPfxSTOtY10L3Teeo+KlQSyM310nnruoRD4mBb+DXX/nRcprlBxyBVrOVK
                J9M2Oh4DVJ2DMR27KtWhlJT5M34d7uf/KH//8LAyNy/8Bob/mWUyo7jkCIKA4DW4QV6ONJRzG2cb
                DAHjgAfbtA8BxgApvPbVfjXVcYY7u0wFQKZzCT+foc7pf7upsjGmc1fDugBvwmZVSrpOgsfgbQ+G
                hfuDuS/mPhlgyfTb+Iq5Zzo2gUl+KH+UX6oJw1Ssgytasyzcn439o/z9PwOrA7lxJo1xAjmHHCX8
                SaJRXcZU2xkFpsIf7qR/L1iFO7QBU3jB0HpNVQiGgpQBk7YJt8H7+d9u5lw11T0SIKRmZJrXMnOt
                htr+b7Hw+yEbfI2138Gm+yN/UluqICVAyQ9/yG/lp6pgjem3+TO+/EPb/m/8+4eH1YH8/dBN1TLj
                EAZcMrV1mfYu01AvcMjR5HByPOOwxjkPpqnwmac/aosxyiDc6fVbsFJaAylTeE0+pgCZ4zTb/yOZ
                gYWAbSoXnasJv8z1MdfkYJn2qfuie6B5LdO+BEgdh+6fKkD5j6Zm3kBqMHzMX7ifGv8MV1Zmebj9
                o/z9Pw0rczPNDf9rbqzSGvVlni6aNhEVjMFgOJim/I0qUEFToZBpn9q/lIPCC6U17Tem0AhM2s4U
                UuUTfsz/SKbzkxlomWugdeac9VvQGnyN/1ZTvkbBKX8dg/Y5WDX9NX/G5/6Sn5o08mnN/6W0/9v+
                /p+Glfn7z95Us1147WZUmEwOaSBmwkc5rAChQmQKlYGaHHywydHDTQVOac12mg5WR2ZbU3AGm9IM
                Ni03+ezPwo/vYNlQ+/kh29+5GDPX15hZZiAtWGl+f+cevr9wM3mZykP3VvfYtDepEjNq3KilvxUk
                B7LNfybf/w1//4TV3/lPDipHNeGjCR0Ht4HJ0QcXrHAbqrCE2+AC9kMAGAwJA0GtG7zPoWyofR8s
                G2p/g03pzLEPPrfB+cmGWj5Uvn8pn/B0g6EkpRQewhlAqbL659/B/fsnrP6L/sIVmFFf4RBTI77g
                FW4qDMZUOIyZkC28EMkGFzIVvHAwycJVmkxKTaa04bD8SzZ4vwfDhtrPUKbzCj+fHwLMD6UbKr3y
                1TU1CincBKZwGwylcMX0t6qmf/795b9/wuq/8c8AzEBMjh9uKgzGVDiMqTYfXIgGFzJT+AZDIbyQ
                yoYqvPuzcAgcLBtqPz9kQx3z4PMcygzwFLqFmyqD8Gs9+D6YsF42GEz//Puv+/snrP4L/0yNayBl
                nN4UgMHrw82klakAhcNLpto+3KTUZFIC4WaePMkEPWMC3uBC/ENmCv3BtKH2M5QZCIeroMHglkmB
                mhDNhGkyc13CTYAKv9bh92Gwmftl7tngbf759/f7+yes/s5/cmDj0OHACXd64/g/9GcKgjGT34Hk
                +5csXDEIeEMV5MEWDr6DbUPtb7Ap3WAADbW9gG2UULgNvlbhNvi6DjZde6UbKr+h0v7z7+D+/RNW
                f+c/Oa5x6L9kSvdDf+Gg+ksWXliMHcjfD237QzbU8f+tNtR+hjKlDYessXCAGIjovA7kT/kOtf1Q
                x2WW/9D6cPvn38H9+yes/s5/4TA50MIzeJuhCsJQZgpQeIHb31/4fv63/B3sY1V+5trJwq9p+PU5
                kP3+NWn/+ffX/f1HWLXh/wf4Cf8QggjKFQAAAABJRU5ErkJggg==" alt="Red dot" />
            <h2>Introduction</h2>
            You have purchased your own do it your self bomb. Welcome to the web page which 
            hosted by your own bomb. Yes, it is an IoT bomb! You have followed the manual and now ready to configuere 
            and setup your bom. Time for action.

            <h2>Be careful</h2>
            Pay attention. Do handle the bomb with care. Any movement can lead to a possible explosion.
            Make sure you are in a calm and non-static environment.
            Remember, it's essential to remain calm. Panic only leads to sweaty hands, and that helps no one!

            <h2>Dismantling</h2>
            What? You should go for your target. Dismantling is a very dangerous action if you want to do it.
            It is possible that you need to dismantle the bomb. It is possible, but very difficult. So, just 
            go for it, but do not wait too long.

            <p><footer>Copyright 2017 (c) htb.jmnl.nl</footer></p>
        </div>

    </center>
</body>
</html>
)rawliteral";

const char admin_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HTB - Admin</title>
    <style>
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            border: 1px solid black;
            padding: 8px;
            text-align: left;
        }
        th {
            background-color: #f2f2f2;
        }
        .warning-box {
            background-color: red;
            color: white;
            padding: 20px;
            text-align: center;
            font-size: 20px;
            font-weight: bold;
            border: 2px solid darkred;
            width: 500px;
            margin: 0 auto;
            border-radius: 10px;
        }
    </style>
</head>
<body>

<h1>Admin page</h1>

<div class="warning-box">
    The bomb is live! Settings cannot be changed! Disable this admin page before using the bomb in a live situation.
</div>
<br>

<table>
    <thead>
        <tr>
            <th>Parameter</th>
            <th>Description</th>
            <th>Options/Range</th>
            <th>Default</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>Countdown Timer</td>
            <td>Sets the countdown duration before the simulated detonation.</td>
            <td>1 to 90 minutes</td>
            <td>30 minutes</td>
        </tr>
        <tr>
            <td>Beep Frequency</td>
            <td>Determines the interval at which the device emits a beeping sound.</td>
            <td>1 second to 30 seconds</td>
            <td>1 seconds</td>
        </tr>
        <tr>
            <td>LED Light Pattern</td>
            <td>Configures the flashing pattern of the LED indicators.</td>
            <td>Steady, Blinking, Pulsing, Random</td>
            <td>Blinking</td>
        </tr>
        <tr>
            <td>Wire Colors</td>
            <td>Sets the color scheme for the device's wires for easy identification.</td>
            <td>Red, Blue, Green; Yellow, Purple, Orange; Black, White, Gray</td>
            <td>Red, Blue, Green</td>
        </tr>
        <tr>
            <td>Activation Code</td>
            <td>Specifies the code required to arm the device.</td>
            <td>4-digit number</td>
            <td>1234</td>
        </tr>
        <tr>
            <td>Deactivation Code</td>
            <td>Specifies the code required to disarm the device. It determines the order of the wires to pull or cut. Make sure it is password protected before using the bomb.</td>
            <td>4-digit hex number</td>
            <td><a href="/code">show secret code</a></td>
        </tr>
        <tr>
            <td>Sound Effects</td>
            <td>Chooses the sound effect played by the device.</td>
            <td>Beep, Buzz, Click, Custom sound (upload required)</td>
            <td>Beep</td>
        </tr>
        <tr>
            <td>Simulated Explosive Force</td>
            <td>Displays the simulated explosive force level.</td>
            <td>Low, Medium, High, Maximum</td>
            <td>Medium</td>
        </tr>
        <tr>
            <td>Error Messages</td>
            <td>Customizes the error messages displayed during malfunction.</td>
            <td>"Incorrect wire!", "Try again.", "Error detected."</td>
            <td>"Incorrect wire!"</td>
        </tr>
        <tr>
            <td>Display Brightness</td>
            <td>Adjusts the brightness of the display screen.</td>
            <td>1 (dim) to 10 (bright)</td>
            <td>5</td>
        </tr>
        <tr>
            <td>Temperature Sensor Sensitivity</td>
            <td>Simulates the sensitivity of the device to temperature changes.</td>
            <td>1 (low sensitivity) to 10 (high sensitivity)</td>
            <td>5</td>
        </tr>
        <tr>
            <td>Voice Activation</td>
            <td>Enables or disables voice command functionality for arming or disarming.</td>
            <td>On, Off</td>
            <td>Off</td>
        </tr>
        <tr>
            <td>Simulated GPS Location</td>
            <td>Sets a simulated GPS location for the device.</td>
            <td>Latitude and Longitude coordinates</td>
            <td>0.0000, 0.0000</td>
        </tr>
        <tr>
            <td>Random Event Generator</td>
            <td>Adds random events during the countdown.</td>
            <td>"Wire switch challenge", "Countdown speed doubled", "Bonus time added"</td>
            <td>Off</td>
        </tr>
    </tbody>
</table>
</body>
</html>
)rawliteral";

const char code_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>HTB</title>
    </head>
    <body>
      <center>
        <p>The unique wire deactivation code for %s:</p>
        <p>%.4s</p>
      </center>
    </body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);

  EEPROM.begin(512);
  uint8_t es = EEPROM.read(0);
  printf("EEPROM(0): '%u'\n", es);
  if ( es != 0x55 ) {
    PASSWORD = "";
    EEPROM.write(0, 0x55);
    for ( uint8_t i=0; i < 20; i++ ) {
      uint8_t r = random(0, 10);
      PASSWORD = PASSWORD + String(r);
      printf("%d\n", r);
      EEPROM.write(i+1, r);
    }
    EEPROM.commit();
  } else {
    PASSWORD = "";
    for ( uint8_t i=0; i < 20; i++ ) {
      PASSWORD = PASSWORD + String(EEPROM.read(i+1));
    }
  }

  // Call the setup to initialize the main drivers.
  for ( IDriver *driver: drivers ) {
      driver->setup();
  }

  printf("%s / %s\n", SSID.c_str(), PASSWORD.c_str());

  WiFi.softAP(SSID, PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/admin", handleAdmin);
  server.on("/code", handleCode);
  server.onNotFound(handleNotFound);
  server.begin();

  timer.blink(true);
  timer.showTime(totalTime, 0);

  Serial.println("Ready.");
}

void loop() {
  for ( IDriver *driver: drivers ) {
      driver->loop(millis());
  }
  
  switch (stateMain) {
    case SELECT_TIME:
      if ( button.isPressed() ) {
        totalTime = (totalTime + 5) % 100;
        timer.showTime(totalTime, 0);
        delay(500);
      }
      if ( button.isLongPressed() ) {
        stateMain = READY;
        timer.blink(false);
        delay(1000);
        button.isPressed(); // reset
      }
    break;

    case READY:
      if ( button.isPressed() ) { 
        stateMain = GAME;
        timer.enterCountdown(totalTime);
      }
    break;

    case GAME:
     if ( wires.isWin() ) {
      stateMain = WIN;
     }
    if ( timer.isTimerZero() || wires.isLose() ) {
      stateMain = LOSE;
     }
    break;

    case WIN:
      timer.showYeah();
      stateMain = END;
    break;

    case LOSE:
      timer.showLose();
      stateMain = END;
    break;

    case END:
      //
    break;

    default:
      stateMain = SELECT_TIME;
  };

  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleAdmin() {
  server.send(200, "text/html", admin_html);
}

void handleCode() {
  char html[1000];
  sprintf(html, code_html, SSID.c_str(), wires.getCode());
  server.send(200, "text/html", html);
}

void handleNotFound () {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
