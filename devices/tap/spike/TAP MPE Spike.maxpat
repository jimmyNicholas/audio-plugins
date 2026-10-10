{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 8,
      "minor": 5,
      "revision": 4,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      100,
      100,
      640,
      420
    ],
    "openrect": [
      0,
      0,
      0,
      169
    ],
    "openinpresentation": 1,
    "default_fontsize": 10,
    "default_fontname": "Arial Bold",
    "gridsize": [
      8,
      8
    ],
    "boxanimatetime": 500,
    "latency": 0,
    "is_mpe": 1,
    "minimum_live_version": "",
    "minimum_max_version": "",
    "platform_compatibility": 0,
    "project": {
      "version": 1,
      "creationdate": 3590052786,
      "modificationdate": 3590052786,
      "viewrect": [
        25,
        106,
        300,
        500
      ],
      "autoorganize": 1,
      "hideprojectwindow": 1,
      "showdependencies": 1,
      "autolocalize": 0,
      "contents": {
        "patchers": {}
      },
      "layout": {},
      "searchpath": {},
      "detailsvisible": 0,
      "amxdtype": 1835887981,
      "readonly": 0,
      "devpathtype": 0,
      "devpath": ".",
      "sortmode": 0,
      "viewmode": 0,
      "includepackages": 0
    },
    "autosave": 0,
    "boxes": [
      {
        "box": {
          "id": "obj-1",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            "int"
          ],
          "patching_rect": [
            24,
            24,
            40,
            20
          ],
          "text": "midiin"
        }
      },
      {
        "box": {
          "id": "obj-2",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            24,
            200,
            88,
            20
          ],
          "text": "js tap-spike.js",
          "saved_object_attributes": {
            "filename": "tap-spike.js",
            "parameter_enable": 0
          }
        }
      },
      {
        "box": {
          "id": "obj-3",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "outlettype": [],
          "patching_rect": [
            24,
            240,
            47,
            20
          ],
          "text": "midiout"
        }
      },
      {
        "box": {
          "id": "obj-4",
          "maxclass": "live.numbox",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            200,
            104,
            44,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            52,
            28,
            44,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_initial": [
                50
              ],
              "parameter_initial_enable": 1,
              "parameter_longname": "Detune",
              "parameter_mmax": 100,
              "parameter_mmin": -100,
              "parameter_modmode": 0,
              "parameter_shortname": "Detune",
              "parameter_type": 1,
              "parameter_unitstyle": 0
            }
          },
          "varname": "Detune"
        }
      },
      {
        "box": {
          "id": "obj-5",
          "maxclass": "live.tab",
          "livemode": 1,
          "num_lines_patching": 1,
          "num_lines_presentation": 1,
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "",
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            320,
            104,
            110,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            52,
            52,
            110,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_enum": [
                "Black keys",
                "All keys"
              ],
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_longname": "Keys",
              "parameter_mmax": 1,
              "parameter_modmode": 0,
              "parameter_shortname": "Keys",
              "parameter_type": 2,
              "parameter_unitstyle": 9
            }
          },
          "varname": "Keys"
        }
      },
      {
        "box": {
          "id": "obj-6",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            200,
            152,
            84,
            20
          ],
          "text": "prepend detune"
        }
      },
      {
        "box": {
          "id": "obj-7",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            320,
            152,
            74,
            20
          ],
          "text": "prepend keys"
        }
      },
      {
        "box": {
          "id": "obj-8",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "bang",
            "int",
            "int"
          ],
          "patching_rect": [
            200,
            24,
            83,
            20
          ],
          "text": "live.thisdevice"
        }
      },
      {
        "box": {
          "id": "obj-9",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "bang",
            "bang"
          ],
          "patching_rect": [
            200,
            64,
            40,
            20
          ],
          "text": "t b b"
        }
      },
      {
        "box": {
          "id": "obj-10",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            24,
            300,
            120,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            4,
            120,
            18
          ],
          "text": "TAP MPE spike"
        }
      },
      {
        "box": {
          "id": "obj-11",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            140,
            104,
            50,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            26,
            48,
            18
          ],
          "text": "Detune"
        }
      },
      {
        "box": {
          "id": "obj-12",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            440,
            104,
            40,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            50,
            48,
            18
          ],
          "text": "Keys"
        }
      },
      {
        "box": {
          "id": "obj-13",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            250,
            104,
            40,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            98,
            26,
            40,
            18
          ],
          "text": "cents"
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "destination": [
            "obj-2",
            0
          ],
          "source": [
            "obj-1",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-3",
            0
          ],
          "source": [
            "obj-2",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-6",
            0
          ],
          "source": [
            "obj-4",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-7",
            0
          ],
          "source": [
            "obj-5",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-2",
            0
          ],
          "source": [
            "obj-6",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-2",
            0
          ],
          "source": [
            "obj-7",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-9",
            0
          ],
          "source": [
            "obj-8",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-4",
            0
          ],
          "source": [
            "obj-9",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-5",
            0
          ],
          "source": [
            "obj-9",
            1
          ]
        }
      }
    ],
    "parameters": {
      "obj-4": [
        "Detune",
        "Detune",
        0
      ],
      "obj-5": [
        "Keys",
        "Keys",
        0
      ],
      "inherited_shortname": 1
    }
  }
}
