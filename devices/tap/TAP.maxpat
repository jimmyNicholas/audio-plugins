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
      360,
      169
    ],
    "devicewidth": 360.0,
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
          "numoutlets": 4,
          "outlettype": [
            "",
            "",
            "",
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
            160,
            18
          ],
          "text": "The Annoying Piano"
        }
      },
      {
        "box": {
          "id": "obj-20",
          "maxclass": "live.slider",
          "varname": "mode",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "orientation": 1,
          "showname": 0,
          "shownumber": 1,
          "parameter_enable": 1,
          "patching_rect": [
            120,
            24,
            110,
            24
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            24,
            200,
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
              ],
              "parameter_steps": 3
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-27",
          "maxclass": "live.dial",
          "varname": "mod",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            184,
            64,
            44,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            48,
            44,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Mod",
              "parameter_shortname": "Mod",
              "parameter_type": 0,
              "parameter_mmin": -12.0,
              "parameter_mmax": 12.0,
              "parameter_initial": [
                1.0
              ],
              "parameter_initial_enable": 1,
              "parameter_steps": 2401,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-34",
          "maxclass": "live.dial",
          "varname": "range",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            640,
            64,
            44,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            72,
            48,
            44,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Effect Range",
              "parameter_shortname": "Range",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 24.0,
              "parameter_initial": [
                0.0
              ],
              "parameter_initial_enable": 1,
              "parameter_steps": 2401,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-35",
          "maxclass": "live.dial",
          "varname": "chance",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            736,
            64,
            44,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            140,
            48,
            44,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Chance",
              "parameter_shortname": "Chance",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 100.0,
              "parameter_initial": [
                100.0
              ],
              "parameter_initial_enable": 1,
              "parameter_steps": 101,
              "parameter_unitstyle": 5
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-32",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "fontname": "Arial",
          "patching_rect": [
            240,
            120,
            44,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            224,
            22,
            60,
            18
          ],
          "text": "Lowest"
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "live.numbox",
          "varname": "lowest",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            240,
            140,
            44,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            224,
            40,
            60,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lowest",
              "parameter_shortname": "Lowest",
              "parameter_type": 1,
              "parameter_mmin": 0,
              "parameter_mmax": 127,
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 8
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-33",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "fontname": "Arial",
          "patching_rect": [
            300,
            120,
            44,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            224,
            60,
            60,
            18
          ],
          "text": "Highest"
        }
      },
      {
        "box": {
          "id": "obj-29",
          "maxclass": "live.numbox",
          "varname": "highest",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "patching_rect": [
            300,
            140,
            44,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            224,
            78,
            60,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Highest",
              "parameter_shortname": "Highest",
              "parameter_type": 1,
              "parameter_mmin": 0,
              "parameter_mmax": 127,
              "parameter_initial": [
                127
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 8
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-45",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            312,
            184,
            72,
            20
          ],
          "text": "prepend mod"
        }
      },
      {
        "box": {
          "id": "obj-48",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            640,
            184,
            80,
            20
          ],
          "text": "prepend range"
        }
      },
      {
        "box": {
          "id": "obj-49",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            736,
            184,
            88,
            20
          ],
          "text": "prepend chance"
        }
      },
      {
        "box": {
          "id": "obj-46",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            120,
            224,
            80,
            20
          ],
          "text": "prepend lowest"
        }
      },
      {
        "box": {
          "id": "obj-47",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            208,
            224,
            88,
            20
          ],
          "text": "prepend highest"
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
            104,
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
            224,
            126,
            120,
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
      },
      {
        "box": {
          "id": "obj-26",
          "maxclass": "live.text",
          "varname": "open",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 0,
          "mode": 0,
          "text": "Open",
          "texton": "Open",
          "patching_rect": [
            24,
            384,
            60,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            56,
            104,
            44,
            15
          ]
        }
      },
      {
        "box": {
          "id": "obj-50",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            24,
            416,
            36,
            20
          ],
          "text": "open"
        }
      },
      {
        "box": {
          "id": "obj-51",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            24,
            448,
            56,
            20
          ],
          "text": "pcontrol"
        }
      },
      {
        "box": {
          "id": "obj-52",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            24,
            480,
            52,
            20
          ],
          "text": "p Panel",
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
              120,
              120,
              920,
              320
            ],
            "openinpresentation": 1,
            "toolbarvisible": 0,
            "enablehscroll": 0,
            "enablevscroll": 0,
            "default_fontsize": 10,
            "default_fontname": "Arial",
            "gridsize": [
              8,
              8
            ],
            "boxanimatetime": 500,
            "autosave": 0,
            "boxes": [
              {
                "box": {
                  "id": "obj-1",
                  "maxclass": "inlet",
                  "comment": "open from pcontrol",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "outlettype": [
                    ""
                  ],
                  "patching_rect": [
                    24,
                    24,
                    30,
                    30
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-2",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "patching_rect": [
                    24,
                    760,
                    72,
                    20
                  ],
                  "text": "thispatcher",
                  "outlettype": [
                    "",
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-3",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    64,
                    60,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    12,
                    100,
                    18
                  ],
                  "text": "Mode",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-4",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    88,
                    60,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    226,
                    100,
                    18
                  ],
                  "text": "Output",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-5",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    112,
                    80,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    152,
                    12,
                    200,
                    18
                  ],
                  "text": "How it works",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-6",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    136,
                    60,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    378,
                    12,
                    200,
                    18
                  ],
                  "text": "Modifiers",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-7",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    160,
                    60,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    378,
                    138,
                    64,
                    18
                  ],
                  "text": "Lowest"
                }
              },
              {
                "box": {
                  "id": "obj-8",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    184,
                    60,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    460,
                    138,
                    64,
                    18
                  ],
                  "text": "Highest"
                }
              },
              {
                "box": {
                  "id": "obj-9",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    208,
                    280,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    84,
                    293,
                    300,
                    18
                  ],
                  "text": "Not saved with the set. Resets when the device loads.",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-10",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    232,
                    100,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    326,
                    200,
                    18
                  ],
                  "text": "Gravity tuning",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-11",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    256,
                    100,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    346,
                    110,
                    18
                  ],
                  "text": "Pull at Mod 1 (%)"
                }
              },
              {
                "box": {
                  "id": "obj-12",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    280,
                    100,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    370,
                    110,
                    18
                  ],
                  "text": "Per doubling (×)"
                }
              },
              {
                "box": {
                  "id": "obj-13",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    304,
                    100,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    240,
                    326,
                    160,
                    18
                  ],
                  "text": "View size",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-14",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    24,
                    328,
                    100,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    432,
                    326,
                    120,
                    18
                  ],
                  "text": "Header summary",
                  "textcolor": [
                    0.66,
                    0.66,
                    0.66,
                    1.0
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-name",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    360,
                    64,
                    210,
                    27
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    152,
                    34,
                    210,
                    27
                  ],
                  "text": "Swap",
                  "varname": "window-name",
                  "fontsize": 18.0
                }
              },
              {
                "box": {
                  "id": "obj-how",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    360,
                    96,
                    210,
                    120
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    152,
                    66,
                    210,
                    160
                  ],
                  "text": "",
                  "varname": "window-how",
                  "fontsize": 12.0
                }
              },
              {
                "box": {
                  "id": "obj-label",
                  "maxclass": "comment",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "fontname": "Arial",
                  "patching_rect": [
                    600,
                    64,
                    232,
                    18
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    378,
                    104,
                    232,
                    18
                  ],
                  "text": "",
                  "varname": "window-label"
                }
              },
              {
                "box": {
                  "id": "obj-15",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    360,
                    24,
                    56,
                    20
                  ],
                  "text": "r ---how",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-16",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 3,
                  "patching_rect": [
                    360,
                    44,
                    90,
                    20
                  ],
                  "text": "route name text",
                  "outlettype": [
                    "",
                    "",
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-17",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    600,
                    24,
                    64,
                    20
                  ],
                  "text": "r ---label",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-mode",
                  "maxclass": "live.slider",
                  "varname": "window-mode",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    "float"
                  ],
                  "orientation": 1,
                  "showname": 0,
                  "shownumber": 1,
                  "parameter_enable": 0,
                  "patching_rect": [
                    24,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    34,
                    120,
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
                      ],
                      "parameter_steps": 3
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-mode-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    24,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---mode-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-mode-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    24,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---mode-window"
                }
              },
              {
                "box": {
                  "id": "obj-mod",
                  "maxclass": "live.dial",
                  "varname": "window-mod",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    136,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    378,
                    34,
                    44,
                    48
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_longname": "Mod",
                      "parameter_shortname": "Mod",
                      "parameter_type": 0,
                      "parameter_mmin": -12.0,
                      "parameter_mmax": 12.0,
                      "parameter_initial": [
                        1.0
                      ],
                      "parameter_initial_enable": 1,
                      "parameter_steps": 2401,
                      "parameter_unitstyle": 1
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-mod-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    136,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---mod-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-mod-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    136,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---mod-window"
                }
              },
              {
                "box": {
                  "id": "obj-range",
                  "maxclass": "live.dial",
                  "varname": "window-range",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    248,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    450,
                    34,
                    44,
                    48
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_longname": "Effect Range",
                      "parameter_shortname": "Range",
                      "parameter_type": 0,
                      "parameter_mmin": 0.0,
                      "parameter_mmax": 24.0,
                      "parameter_initial": [
                        0.0
                      ],
                      "parameter_initial_enable": 1,
                      "parameter_steps": 2401,
                      "parameter_unitstyle": 1
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-range-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    248,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---range-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-range-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    248,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---range-window"
                }
              },
              {
                "box": {
                  "id": "obj-chance",
                  "maxclass": "live.dial",
                  "varname": "window-chance",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    360,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    522,
                    34,
                    44,
                    48
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_longname": "Chance",
                      "parameter_shortname": "Chance",
                      "parameter_type": 0,
                      "parameter_mmin": 0.0,
                      "parameter_mmax": 100.0,
                      "parameter_initial": [
                        100.0
                      ],
                      "parameter_initial_enable": 1,
                      "parameter_steps": 101,
                      "parameter_unitstyle": 5
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-chance-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    360,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---chance-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-chance-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    360,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---chance-window"
                }
              },
              {
                "box": {
                  "id": "obj-lowest",
                  "maxclass": "live.numbox",
                  "varname": "window-lowest",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    472,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    378,
                    156,
                    64,
                    15
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_longname": "Lowest",
                      "parameter_shortname": "Lowest",
                      "parameter_type": 1,
                      "parameter_mmin": 0,
                      "parameter_mmax": 127,
                      "parameter_initial": [
                        0
                      ],
                      "parameter_initial_enable": 1,
                      "parameter_unitstyle": 8
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-lowest-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    472,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---lowest-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-lowest-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    472,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---lowest-window"
                }
              },
              {
                "box": {
                  "id": "obj-highest",
                  "maxclass": "live.numbox",
                  "varname": "window-highest",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    584,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    460,
                    156,
                    64,
                    15
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_longname": "Highest",
                      "parameter_shortname": "Highest",
                      "parameter_type": 1,
                      "parameter_mmin": 0,
                      "parameter_mmax": 127,
                      "parameter_initial": [
                        127
                      ],
                      "parameter_initial_enable": 1,
                      "parameter_unitstyle": 8
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-highest-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    584,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---highest-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-highest-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    584,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---highest-window"
                }
              },
              {
                "box": {
                  "id": "obj-output",
                  "maxclass": "live.tab",
                  "varname": "window-output",
                  "numinlets": 1,
                  "numoutlets": 3,
                  "outlettype": [
                    "",
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "num_lines_patching": 1,
                  "num_lines_presentation": 1,
                  "patching_rect": [
                    696,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    246,
                    120,
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
                  "id": "obj-output-r",
                  "maxclass": "newobj",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "patching_rect": [
                    696,
                    408,
                    96,
                    20
                  ],
                  "text": "r ---output-strip",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-output-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    696,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---output-window"
                }
              },
              {
                "box": {
                  "id": "obj-reset",
                  "maxclass": "live.text",
                  "varname": "window-reset",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    ""
                  ],
                  "parameter_enable": 0,
                  "mode": 0,
                  "text": "Reset",
                  "texton": "Reset",
                  "patching_rect": [
                    808,
                    440,
                    60,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    196,
                    60,
                    16
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
                  "id": "obj-reset-sel",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "patching_rect": [
                    808,
                    472,
                    40,
                    20
                  ],
                  "text": "sel 1",
                  "outlettype": [
                    "",
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-reset-s",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    808,
                    504,
                    104,
                    20
                  ],
                  "text": "s ---reset-window"
                }
              },
              {
                "box": {
                  "id": "obj-18",
                  "maxclass": "panel",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "mode": 0,
                  "bgcolor": [
                    0.0,
                    0.0,
                    0.0,
                    1.0
                  ],
                  "border": 0,
                  "patching_rect": [
                    640,
                    64,
                    128,
                    128
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    648,
                    12,
                    256,
                    256
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-debug",
                  "maxclass": "live.text",
                  "varname": "window-debug",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    ""
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    24,
                    600,
                    60,
                    16
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    16,
                    294,
                    60,
                    16
                  ],
                  "mode": 1,
                  "text": "Debug",
                  "texton": "Debug"
                }
              },
              {
                "box": {
                  "id": "obj-19",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 3,
                  "patching_rect": [
                    24,
                    632,
                    52,
                    20
                  ],
                  "text": "sel 0 1",
                  "outlettype": [
                    "",
                    "",
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-20",
                  "maxclass": "message",
                  "numinlets": 2,
                  "numoutlets": 1,
                  "outlettype": [
                    ""
                  ],
                  "patching_rect": [
                    24,
                    664,
                    240,
                    20
                  ],
                  "text": "window size 120 120 1040 440, window exec"
                }
              },
              {
                "box": {
                  "id": "obj-21",
                  "maxclass": "message",
                  "numinlets": 2,
                  "numoutlets": 1,
                  "outlettype": [
                    ""
                  ],
                  "patching_rect": [
                    24,
                    696,
                    240,
                    20
                  ],
                  "text": "window size 120 120 1040 550, window exec"
                }
              },
              {
                "box": {
                  "id": "obj-pull",
                  "maxclass": "live.numbox",
                  "varname": "window-pull",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    ""
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    320,
                    600,
                    44,
                    15
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    130,
                    348,
                    56,
                    15
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_type": 0,
                      "parameter_mmin": 0.0,
                      "parameter_mmax": 100.0,
                      "parameter_unitstyle": 1
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-doubling",
                  "maxclass": "live.numbox",
                  "varname": "window-doubling",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    ""
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    400,
                    600,
                    44,
                    15
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    130,
                    372,
                    56,
                    15
                  ],
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_type": 0,
                      "parameter_mmin": 1.0,
                      "parameter_mmax": 10.0,
                      "parameter_unitstyle": 1
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-size",
                  "maxclass": "live.tab",
                  "varname": "window-size",
                  "numinlets": 1,
                  "numoutlets": 3,
                  "outlettype": [
                    "",
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    480,
                    600,
                    160,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    240,
                    346,
                    160,
                    20
                  ],
                  "num_lines_patching": 1,
                  "num_lines_presentation": 1,
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_type": 2,
                      "parameter_mmin": 0,
                      "parameter_mmax": 3,
                      "parameter_unitstyle": 9,
                      "parameter_enum": [
                        "128",
                        "256",
                        "384",
                        "512"
                      ]
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-summary",
                  "maxclass": "live.tab",
                  "varname": "window-summary",
                  "numinlets": 1,
                  "numoutlets": 3,
                  "outlettype": [
                    "",
                    "",
                    "float"
                  ],
                  "parameter_enable": 0,
                  "patching_rect": [
                    680,
                    600,
                    80,
                    20
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    432,
                    346,
                    80,
                    20
                  ],
                  "num_lines_patching": 1,
                  "num_lines_presentation": 1,
                  "saved_attribute_attributes": {
                    "valueof": {
                      "parameter_type": 2,
                      "parameter_mmin": 0,
                      "parameter_mmax": 1,
                      "parameter_unitstyle": 9,
                      "parameter_enum": [
                        "Off",
                        "On"
                      ]
                    }
                  }
                }
              },
              {
                "box": {
                  "id": "obj-22",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    320,
                    568,
                    88,
                    20
                  ],
                  "text": "loadmess set 1",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-23",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    416,
                    568,
                    88,
                    20
                  ],
                  "text": "loadmess set 3",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-24",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    512,
                    568,
                    88,
                    20
                  ],
                  "text": "loadmess set 1",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-25",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    320,
                    632,
                    120,
                    20
                  ],
                  "text": "prepend gravityAtOne",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-26",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    320,
                    664,
                    150,
                    20
                  ],
                  "text": "prepend gravityPerDoubling",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-27",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    320,
                    696,
                    72,
                    20
                  ],
                  "text": "s ---tuning"
                }
              },
              {
                "box": {
                  "id": "obj-28",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    480,
                    632,
                    76,
                    20
                  ],
                  "text": "prepend size",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-29",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "patching_rect": [
                    680,
                    632,
                    96,
                    20
                  ],
                  "text": "prepend summary",
                  "outlettype": [
                    ""
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-30",
                  "maxclass": "newobj",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    560,
                    696,
                    88,
                    20
                  ],
                  "text": "s ---visualiser"
                }
              }
            ],
            "lines": [
              {
                "patchline": {
                  "source": [
                    "obj-15",
                    0
                  ],
                  "destination": [
                    "obj-16",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-16",
                    0
                  ],
                  "destination": [
                    "obj-name",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-16",
                    1
                  ],
                  "destination": [
                    "obj-how",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-17",
                    0
                  ],
                  "destination": [
                    "obj-label",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-mode-r",
                    0
                  ],
                  "destination": [
                    "obj-mode",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-mode",
                    0
                  ],
                  "destination": [
                    "obj-mode-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-mod-r",
                    0
                  ],
                  "destination": [
                    "obj-mod",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-mod",
                    0
                  ],
                  "destination": [
                    "obj-mod-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-range-r",
                    0
                  ],
                  "destination": [
                    "obj-range",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-range",
                    0
                  ],
                  "destination": [
                    "obj-range-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-chance-r",
                    0
                  ],
                  "destination": [
                    "obj-chance",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-chance",
                    0
                  ],
                  "destination": [
                    "obj-chance-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-lowest-r",
                    0
                  ],
                  "destination": [
                    "obj-lowest",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-lowest",
                    0
                  ],
                  "destination": [
                    "obj-lowest-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-highest-r",
                    0
                  ],
                  "destination": [
                    "obj-highest",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-highest",
                    0
                  ],
                  "destination": [
                    "obj-highest-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-output-r",
                    0
                  ],
                  "destination": [
                    "obj-output",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-output",
                    0
                  ],
                  "destination": [
                    "obj-output-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-reset",
                    0
                  ],
                  "destination": [
                    "obj-reset-sel",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-reset-sel",
                    0
                  ],
                  "destination": [
                    "obj-reset-s",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-debug",
                    0
                  ],
                  "destination": [
                    "obj-19",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-19",
                    0
                  ],
                  "destination": [
                    "obj-20",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-19",
                    1
                  ],
                  "destination": [
                    "obj-21",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-20",
                    0
                  ],
                  "destination": [
                    "obj-2",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-21",
                    0
                  ],
                  "destination": [
                    "obj-2",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-22",
                    0
                  ],
                  "destination": [
                    "obj-pull",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-23",
                    0
                  ],
                  "destination": [
                    "obj-doubling",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-24",
                    0
                  ],
                  "destination": [
                    "obj-size",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-pull",
                    0
                  ],
                  "destination": [
                    "obj-25",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-doubling",
                    0
                  ],
                  "destination": [
                    "obj-26",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-25",
                    0
                  ],
                  "destination": [
                    "obj-27",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-26",
                    0
                  ],
                  "destination": [
                    "obj-27",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-size",
                    0
                  ],
                  "destination": [
                    "obj-28",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-summary",
                    0
                  ],
                  "destination": [
                    "obj-29",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-28",
                    0
                  ],
                  "destination": [
                    "obj-30",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-29",
                    0
                  ],
                  "destination": [
                    "obj-30",
                    0
                  ]
                }
              }
            ]
          }
        }
      },
      {
        "box": {
          "id": "obj-53",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            120,
            304,
            72,
            20
          ],
          "text": "s ---drift"
        }
      },
      {
        "box": {
          "id": "obj-60",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "fontname": "Arial",
          "patching_rect": [
            528,
            0,
            60,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            224,
            108,
            60,
            18
          ],
          "text": "Output"
        }
      },
      {
        "box": {
          "id": "obj-61",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            24,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-62",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            24,
            592,
            96,
            20
          ],
          "text": "s ---mode-strip"
        }
      },
      {
        "box": {
          "id": "obj-63",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            24,
            528,
            104,
            20
          ],
          "text": "r ---mode-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-64",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            136,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-65",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            136,
            592,
            96,
            20
          ],
          "text": "s ---mod-strip"
        }
      },
      {
        "box": {
          "id": "obj-66",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            136,
            528,
            104,
            20
          ],
          "text": "r ---mod-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-67",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            248,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-68",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            248,
            592,
            96,
            20
          ],
          "text": "s ---range-strip"
        }
      },
      {
        "box": {
          "id": "obj-69",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            248,
            528,
            104,
            20
          ],
          "text": "r ---range-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-70",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            360,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-71",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            360,
            592,
            96,
            20
          ],
          "text": "s ---chance-strip"
        }
      },
      {
        "box": {
          "id": "obj-72",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            360,
            528,
            104,
            20
          ],
          "text": "r ---chance-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-73",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            472,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-74",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            472,
            592,
            96,
            20
          ],
          "text": "s ---lowest-strip"
        }
      },
      {
        "box": {
          "id": "obj-75",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            472,
            528,
            104,
            20
          ],
          "text": "r ---lowest-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-76",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            584,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-77",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            584,
            592,
            96,
            20
          ],
          "text": "s ---highest-strip"
        }
      },
      {
        "box": {
          "id": "obj-78",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            584,
            528,
            104,
            20
          ],
          "text": "r ---highest-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-79",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            696,
            560,
            72,
            20
          ],
          "text": "prepend set",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-80",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            696,
            592,
            96,
            20
          ],
          "text": "s ---output-strip"
        }
      },
      {
        "box": {
          "id": "obj-81",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            696,
            528,
            104,
            20
          ],
          "text": "r ---output-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-84",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            808,
            528,
            104,
            20
          ],
          "text": "r ---reset-window",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-90",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            240,
            304,
            64,
            20
          ],
          "text": "s ---label"
        }
      },
      {
        "box": {
          "id": "obj-91",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            312,
            304,
            56,
            20
          ],
          "text": "s ---how"
        }
      },
      {
        "box": {
          "id": "obj-92",
          "maxclass": "newobj",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            120,
            224,
            72,
            20
          ],
          "text": "r ---tuning",
          "outlettype": [
            ""
          ]
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
      },
      {
        "patchline": {
          "destination": [
            "obj-53",
            0
          ],
          "source": [
            "obj-2",
            1
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-50",
            0
          ],
          "source": [
            "obj-26",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-51",
            0
          ],
          "source": [
            "obj-50",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-52",
            0
          ],
          "source": [
            "obj-51",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-45",
            0
          ],
          "source": [
            "obj-27",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-46",
            0
          ],
          "source": [
            "obj-28",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-47",
            0
          ],
          "source": [
            "obj-29",
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
            "obj-45",
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
            "obj-46",
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
            "obj-47",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-48",
            0
          ],
          "source": [
            "obj-34",
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
            "obj-48",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-49",
            0
          ],
          "source": [
            "obj-35",
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
            "obj-49",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-20",
            0
          ],
          "destination": [
            "obj-61",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-61",
            0
          ],
          "destination": [
            "obj-62",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-20",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-63",
            0
          ],
          "destination": [
            "obj-20",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-27",
            0
          ],
          "destination": [
            "obj-64",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-64",
            0
          ],
          "destination": [
            "obj-65",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-27",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-66",
            0
          ],
          "destination": [
            "obj-27",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-34",
            0
          ],
          "destination": [
            "obj-67",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-67",
            0
          ],
          "destination": [
            "obj-68",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-34",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-69",
            0
          ],
          "destination": [
            "obj-34",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-35",
            0
          ],
          "destination": [
            "obj-70",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-70",
            0
          ],
          "destination": [
            "obj-71",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-35",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-72",
            0
          ],
          "destination": [
            "obj-35",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-28",
            0
          ],
          "destination": [
            "obj-73",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-73",
            0
          ],
          "destination": [
            "obj-74",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-28",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-75",
            0
          ],
          "destination": [
            "obj-28",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-29",
            0
          ],
          "destination": [
            "obj-76",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-76",
            0
          ],
          "destination": [
            "obj-77",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-29",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-78",
            0
          ],
          "destination": [
            "obj-29",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            0
          ],
          "destination": [
            "obj-79",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-79",
            0
          ],
          "destination": [
            "obj-80",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-30",
            0
          ],
          "destination": [
            "obj-25",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-81",
            0
          ],
          "destination": [
            "obj-25",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-84",
            0
          ],
          "destination": [
            "obj-24",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-2",
            2
          ],
          "destination": [
            "obj-90",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-2",
            3
          ],
          "destination": [
            "obj-91",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-92",
            0
          ],
          "destination": [
            "obj-2",
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
      "obj-27": [
        "Mod",
        "Mod",
        0
      ],
      "obj-28": [
        "Lowest",
        "Lowest",
        0
      ],
      "obj-29": [
        "Highest",
        "Highest",
        0
      ],
      "obj-34": [
        "Effect Range",
        "Range",
        0
      ],
      "obj-35": [
        "Chance",
        "Chance",
        0
      ],
      "inherited_shortname": 1
    }
  }
}
