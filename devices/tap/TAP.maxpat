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
      400
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
            264,
            52,
            20
          ],
          "text": "js tap.js",
          "saved_object_attributes": {
            "filename": "tap.js",
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
            304,
            47,
            20
          ],
          "text": "midiout"
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
            344,
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
          "text": "The Annoying Piano"
        }
      },
      {
        "box": {
          "id": "obj-20",
          "maxclass": "live.tab",
          "varname": "mode",
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "",
            "",
            "float"
          ],
          "parameter_enable": 1,
          "num_lines_patching": 1,
          "num_lines_presentation": 1,
          "patching_rect": [
            120,
            24,
            150,
            20
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            26,
            150,
            20
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Mode",
              "parameter_shortname": "Mode",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 2,
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 9,
              "parameter_enum": [
                "Swap",
                "Gravity",
                "Move"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-21",
          "maxclass": "live.dial",
          "varname": "strength",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            120,
            64,
            44,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            56,
            44,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Strength",
              "parameter_shortname": "Strength",
              "parameter_type": 0,
              "parameter_mmin": 1.0,
              "parameter_mmax": 10.0,
              "parameter_initial": [
                2.0
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-22",
          "maxclass": "live.numbox",
          "varname": "semitones",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            184,
            80,
            44,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            60,
            72,
            44,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Semitones",
              "parameter_shortname": "Semitones",
              "parameter_type": 1,
              "parameter_mmin": -2,
              "parameter_mmax": 2,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 0
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-23",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "fontname": "Arial",
          "patching_rect": [
            240,
            80,
            60,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            56,
            56,
            60,
            18
          ],
          "text": "Semitones"
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "live.text",
          "varname": "reset",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Reset",
          "texton": "Reset",
          "patching_rect": [
            320,
            64,
            44,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            112,
            44,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Reset",
              "parameter_shortname": "Reset",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 9,
              "parameter_enum": [
                "off",
                "on"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "bang",
            "int",
            "int"
          ],
          "patching_rect": [
            440,
            24,
            85,
            20
          ],
          "text": "live.thisdevice"
        }
      },
      {
        "box": {
          "id": "obj-31",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            440,
            64,
            34,
            20
          ],
          "text": "sync"
        }
      },
      {
        "box": {
          "id": "obj-40",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            120,
            184,
            80,
            20
          ],
          "text": "prepend mode"
        }
      },
      {
        "box": {
          "id": "obj-41",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            208,
            184,
            96,
            20
          ],
          "text": "prepend strength"
        }
      },
      {
        "box": {
          "id": "obj-42",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            312,
            184,
            104,
            20
          ],
          "text": "prepend semitones"
        }
      },
      {
        "box": {
          "id": "obj-43",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            424,
            184,
            80,
            20
          ],
          "text": "prepend reset"
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "live.tab",
          "varname": "output",
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "",
            "",
            "float"
          ],
          "parameter_enable": 1,
          "num_lines_patching": 1,
          "num_lines_presentation": 1,
          "patching_rect": [
            528,
            24,
            110,
            20
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            136,
            110,
            20
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Output",
              "parameter_shortname": "Output",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 9,
              "parameter_enum": [
                "MPE",
                "Semitone"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-44",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            528,
            184,
            88,
            20
          ],
          "text": "prepend output"
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
            "obj-40",
            0
          ],
          "source": [
            "obj-20",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-41",
            0
          ],
          "source": [
            "obj-21",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-42",
            0
          ],
          "source": [
            "obj-22",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-43",
            0
          ],
          "source": [
            "obj-24",
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
            "obj-40",
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
            "obj-41",
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
            "obj-42",
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
            "obj-43",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-31",
            0
          ],
          "source": [
            "obj-30",
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
            "obj-31",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-44",
            0
          ],
          "source": [
            "obj-25",
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
            "obj-44",
            0
          ]
        }
      }
    ],
    "parameters": {
      "obj-20": [
        "Mode",
        "Mode",
        0
      ],
      "obj-21": [
        "Strength",
        "Strength",
        0
      ],
      "obj-22": [
        "Semitones",
        "Semitones",
        0
      ],
      "obj-24": [
        "Reset",
        "Reset",
        0
      ],
      "obj-25": [
        "Output",
        "Output",
        0
      ],
      "inherited_shortname": 1
    }
  }
}
