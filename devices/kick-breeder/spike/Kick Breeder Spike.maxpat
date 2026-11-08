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
      1500,
      1000
    ],
    "openrect": [
      0,
      0,
      686,
      169
    ],
    "devicewidth": 686,
    "openinpresentation": 1,
    "default_fontsize": 10,
    "default_fontname": "Arial Bold",
    "gridsize": [
      8,
      8
    ],
    "boxanimatetime": 500,
    "latency": 0,
    "is_mpe": 0,
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
      "amxdtype": 1768515945,
      "readonly": 0,
      "devpathtype": 0,
      "devpath": ".",
      "sortmode": 0,
      "viewmode": 0,
      "includepackages": 0
    },
    "autosave": 0,
    "parameters": {
      "inherited_shortname": 1,
      "obj-10": [
        "Mode",
        "Mode",
        0
      ],
      "obj-11": [
        "Match Loudness",
        "Match Loudness",
        0
      ],
      "obj-12": [
        "Shuffle Slots",
        "Shuffle Slots",
        0
      ],
      "obj-13": [
        "Show Favourite",
        "Show Favourite",
        0
      ],
      "obj-14": [
        "Play",
        "Play",
        0
      ],
      "obj-17": [
        "Hear A",
        "Hear A",
        0
      ],
      "obj-18": [
        "Choose A",
        "Choose A",
        0
      ],
      "obj-21": [
        "Hear B",
        "Hear B",
        0
      ],
      "obj-22": [
        "Choose B",
        "Choose B",
        0
      ],
      "obj-25": [
        "Hear C",
        "Hear C",
        0
      ],
      "obj-26": [
        "Choose C",
        "Choose C",
        0
      ],
      "obj-28": [
        "Wider",
        "Wider",
        0
      ],
      "obj-29": [
        "Closer",
        "Closer",
        0
      ],
      "obj-30": [
        "Back",
        "Back",
        0
      ],
      "obj-31": [
        "Fresh",
        "Fresh",
        0
      ],
      "obj-32": [
        "Kept Pitch",
        "Pitch",
        0
      ],
      "obj-33": [
        "Lock Pitch",
        "Lock Pitch",
        0
      ],
      "obj-34": [
        "Kept Sweep",
        "Sweep",
        0
      ],
      "obj-35": [
        "Lock Sweep",
        "Lock Sweep",
        0
      ],
      "obj-36": [
        "Kept Sweep Time",
        "Sweep T",
        0
      ],
      "obj-37": [
        "Lock Sweep T",
        "Lock Sweep T",
        0
      ],
      "obj-38": [
        "Kept Sweep Curve",
        "Curve",
        0
      ],
      "obj-39": [
        "Lock Curve",
        "Lock Curve",
        0
      ],
      "obj-40": [
        "Kept Decay",
        "Decay",
        0
      ],
      "obj-41": [
        "Lock Decay",
        "Lock Decay",
        0
      ],
      "obj-42": [
        "Kept Click",
        "Click",
        0
      ],
      "obj-43": [
        "Lock Click",
        "Lock Click",
        0
      ],
      "obj-44": [
        "Kept Click Tone",
        "Click T",
        0
      ],
      "obj-45": [
        "Lock Click T",
        "Lock Click T",
        0
      ],
      "obj-46": [
        "Kept Drive",
        "Drive",
        0
      ],
      "obj-47": [
        "Lock Drive",
        "Lock Drive",
        0
      ],
      "obj-48": [
        "Kept Tone",
        "Tone",
        0
      ],
      "obj-49": [
        "Lock Tone",
        "Lock Tone",
        0
      ],
      "obj-50": [
        "Keep",
        "Keep",
        0
      ],
      "obj-51": [
        "Breed From Kept",
        "Breed From Kept",
        0
      ]
    },
    "boxes": [
      {
        "box": {
          "id": "obj-1",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            20,
            20,
            420,
            22
          ],
          "text": "Kick Breeder spike. Throwaway. See README.md. The js keeps the breeding state and works out each kick's envelopes."
        }
      },
      {
        "box": {
          "id": "obj-2",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 5,
          "outlettype": [
            "",
            "",
            "",
            "",
            ""
          ],
          "patching_rect": [
            20,
            50,
            118,
            22
          ],
          "text": "js kick-breeder.js",
          "saved_object_attributes": {
            "filename": "kick-breeder.js",
            "parameter_enable": 0
          }
        }
      },
      {
        "box": {
          "id": "obj-3",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "bang",
            "int",
            "int"
          ],
          "patching_rect": [
            20,
            80,
            100,
            22
          ],
          "text": "live.thisdevice"
        }
      },
      {
        "box": {
          "id": "obj-4",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            20,
            110,
            58,
            22
          ],
          "text": "deferlow"
        }
      },
      {
        "box": {
          "id": "obj-5",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            20,
            140,
            60,
            22
          ],
          "text": "init"
        }
      },
      {
        "box": {
          "id": "obj-6",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            900,
            20,
            230,
            16
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            2,
            230,
            16
          ],
          "text": "KICK BREEDER SPIKE",
          "fontsize": 11
        }
      },
      {
        "box": {
          "id": "obj-7",
          "maxclass": "comment",
          "varname": "status",
          "fontname": "Arial",
          "fontsize": 9,
          "linecount": 2,
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            900,
            44,
            230,
            24
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            20,
            230,
            24
          ],
          "text": "Loading..."
        }
      },
      {
        "box": {
          "id": "obj-8",
          "maxclass": "comment",
          "varname": "detail",
          "fontname": "Arial",
          "fontsize": 9,
          "linecount": 2,
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            900,
            76,
            230,
            24
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            46,
            230,
            24
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-9",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            900,
            108,
            34,
            16
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            76,
            34,
            16
          ],
          "text": "Mode"
        }
      },
      {
        "box": {
          "id": "obj-10",
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
            900,
            132,
            120,
            16
          ],
          "presentation": 1,
          "presentation_rect": [
            40,
            76,
            120,
            16
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Mode",
              "parameter_shortname": "Mode",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
              "parameter_unitstyle": 9,
              "parameter_enum": [
                "Breed",
                "Kept"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-11",
          "maxclass": "live.text",
          "varname": "match",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Match level",
          "texton": "Match level",
          "patching_rect": [
            900,
            156,
            74,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            100,
            74,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Match Loudness",
              "parameter_shortname": "Match Loudness",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                1
              ],
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
          "id": "obj-12",
          "maxclass": "live.text",
          "varname": "shuffle",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Shuffle",
          "texton": "Shuffle",
          "patching_rect": [
            900,
            179,
            74,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            82,
            100,
            74,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Shuffle Slots",
              "parameter_shortname": "Shuffle Slots",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                1
              ],
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
          "id": "obj-13",
          "maxclass": "live.text",
          "varname": "showfav",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Show fav",
          "texton": "Show fav",
          "patching_rect": [
            900,
            202,
            74,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            160,
            100,
            74,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Show Favourite",
              "parameter_shortname": "Show Favourite",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-14",
          "maxclass": "live.text",
          "varname": "play_kick",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Play",
          "texton": "Play",
          "patching_rect": [
            900,
            225,
            74,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            122,
            74,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Play",
              "parameter_shortname": "Play",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-15",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            900,
            248,
            230,
            26
          ],
          "presentation": 1,
          "presentation_rect": [
            4,
            140,
            230,
            26
          ],
          "text": "Hear plays a slot. MIDI notes play the slot you last heard, or the kept kick.",
          "linecount": 2,
          "fontsize": 9
        }
      },
      {
        "box": {
          "id": "obj-16",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1150,
            20,
            18,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            242,
            2,
            18,
            18
          ],
          "text": "A",
          "fontsize": 14
        }
      },
      {
        "box": {
          "id": "obj-17",
          "maxclass": "live.text",
          "varname": "hear_a",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Hear A",
          "texton": "Hear A",
          "patching_rect": [
            1150,
            46,
            56,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            262,
            2,
            56,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Hear A",
              "parameter_shortname": "Hear A",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-18",
          "maxclass": "live.text",
          "varname": "choose_a",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Choose A",
          "texton": "Choose A",
          "patching_rect": [
            1150,
            69,
            60,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            322,
            2,
            60,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Choose A",
              "parameter_shortname": "Choose A",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-19",
          "maxclass": "comment",
          "varname": "fav_0",
          "fontname": "Arial",
          "fontsize": 9,
          "linecount": 1,
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1150,
            92,
            120,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            262,
            19,
            120,
            13
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-20",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1150,
            113,
            18,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            242,
            36,
            18,
            18
          ],
          "text": "B",
          "fontsize": 14
        }
      },
      {
        "box": {
          "id": "obj-21",
          "maxclass": "live.text",
          "varname": "hear_b",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Hear B",
          "texton": "Hear B",
          "patching_rect": [
            1150,
            139,
            56,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            262,
            36,
            56,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Hear B",
              "parameter_shortname": "Hear B",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-22",
          "maxclass": "live.text",
          "varname": "choose_b",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Choose B",
          "texton": "Choose B",
          "patching_rect": [
            1150,
            162,
            60,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            322,
            36,
            60,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Choose B",
              "parameter_shortname": "Choose B",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-23",
          "maxclass": "comment",
          "varname": "fav_1",
          "fontname": "Arial",
          "fontsize": 9,
          "linecount": 1,
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1150,
            185,
            120,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            262,
            53,
            120,
            13
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1150,
            206,
            18,
            18
          ],
          "presentation": 1,
          "presentation_rect": [
            242,
            70,
            18,
            18
          ],
          "text": "C",
          "fontsize": 14
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "live.text",
          "varname": "hear_c",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Hear C",
          "texton": "Hear C",
          "patching_rect": [
            1150,
            232,
            56,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            262,
            70,
            56,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Hear C",
              "parameter_shortname": "Hear C",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-26",
          "maxclass": "live.text",
          "varname": "choose_c",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Choose C",
          "texton": "Choose C",
          "patching_rect": [
            1150,
            255,
            60,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            322,
            70,
            60,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Choose C",
              "parameter_shortname": "Choose C",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-27",
          "maxclass": "comment",
          "varname": "fav_2",
          "fontname": "Arial",
          "fontsize": 9,
          "linecount": 1,
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1150,
            278,
            120,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            262,
            87,
            120,
            13
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "live.text",
          "varname": "wider",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Wider",
          "texton": "Wider",
          "patching_rect": [
            1150,
            299,
            68,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            242,
            106,
            68,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Wider",
              "parameter_shortname": "Wider",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-29",
          "maxclass": "live.text",
          "varname": "closer",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Closer",
          "texton": "Closer",
          "patching_rect": [
            1150,
            322,
            68,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            314,
            106,
            68,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Closer",
              "parameter_shortname": "Closer",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "maxclass": "live.text",
          "varname": "back",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Back",
          "texton": "Back",
          "patching_rect": [
            1150,
            345,
            68,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            242,
            126,
            68,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Back",
              "parameter_shortname": "Back",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-31",
          "maxclass": "live.text",
          "varname": "fresh",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Fresh",
          "texton": "Fresh",
          "patching_rect": [
            1150,
            368,
            68,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            314,
            126,
            68,
            15
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Fresh",
              "parameter_shortname": "Fresh",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-32",
          "maxclass": "live.dial",
          "varname": "kept_0",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Pitch",
          "patching_rect": [
            1400,
            20,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            392,
            2,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Pitch",
              "parameter_shortname": "Pitch",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 30,
              "parameter_mmax": 120,
              "parameter_initial": [
                55
              ],
              "parameter_unitstyle": 3,
              "parameter_exponent": 2
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-33",
          "maxclass": "live.text",
          "varname": "lock_0",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            72,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            392,
            48,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Pitch",
              "parameter_shortname": "Lock Pitch",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-34",
          "maxclass": "live.dial",
          "varname": "kept_1",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Sweep",
          "patching_rect": [
            1400,
            93,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            438,
            2,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Sweep",
              "parameter_shortname": "Sweep",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 48,
              "parameter_initial": [
                24
              ],
              "parameter_unitstyle": 7
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-35",
          "maxclass": "live.text",
          "varname": "lock_1",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            145,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            438,
            48,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Sweep",
              "parameter_shortname": "Lock Sweep",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-36",
          "maxclass": "live.dial",
          "varname": "kept_2",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Sweep T",
          "patching_rect": [
            1400,
            166,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            484,
            2,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Sweep Time",
              "parameter_shortname": "Sweep T",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 5,
              "parameter_mmax": 200,
              "parameter_initial": [
                40
              ],
              "parameter_unitstyle": 2,
              "parameter_exponent": 3
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-37",
          "maxclass": "live.text",
          "varname": "lock_2",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            218,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            484,
            48,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Sweep T",
              "parameter_shortname": "Lock Sweep T",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-38",
          "maxclass": "live.dial",
          "varname": "kept_3",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Curve",
          "patching_rect": [
            1400,
            239,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            530,
            2,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Sweep Curve",
              "parameter_shortname": "Curve",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0.5
              ],
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-39",
          "maxclass": "live.text",
          "varname": "lock_3",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            291,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            530,
            48,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Curve",
              "parameter_shortname": "Lock Curve",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-40",
          "maxclass": "live.dial",
          "varname": "kept_4",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Decay",
          "patching_rect": [
            1400,
            312,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            576,
            2,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Decay",
              "parameter_shortname": "Decay",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 50,
              "parameter_mmax": 1500,
              "parameter_initial": [
                400
              ],
              "parameter_unitstyle": 2,
              "parameter_exponent": 3
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-41",
          "maxclass": "live.text",
          "varname": "lock_4",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            364,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            576,
            48,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Decay",
              "parameter_shortname": "Lock Decay",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-42",
          "maxclass": "live.dial",
          "varname": "kept_5",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Click",
          "patching_rect": [
            1400,
            385,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            392,
            64,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Click",
              "parameter_shortname": "Click",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0.3
              ],
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-43",
          "maxclass": "live.text",
          "varname": "lock_5",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            437,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            392,
            110,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Click",
              "parameter_shortname": "Lock Click",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-44",
          "maxclass": "live.dial",
          "varname": "kept_6",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Click T",
          "patching_rect": [
            1400,
            458,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            438,
            64,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Click Tone",
              "parameter_shortname": "Click T",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 500,
              "parameter_mmax": 12000,
              "parameter_initial": [
                3000
              ],
              "parameter_unitstyle": 3,
              "parameter_exponent": 3
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-45",
          "maxclass": "live.text",
          "varname": "lock_6",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            510,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            438,
            110,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Click T",
              "parameter_shortname": "Lock Click T",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-46",
          "maxclass": "live.dial",
          "varname": "kept_7",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Drive",
          "patching_rect": [
            1400,
            531,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            484,
            64,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Drive",
              "parameter_shortname": "Drive",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0.2
              ],
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-47",
          "maxclass": "live.text",
          "varname": "lock_7",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            583,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            484,
            110,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Drive",
              "parameter_shortname": "Lock Drive",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-48",
          "maxclass": "live.dial",
          "varname": "kept_8",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "parameter_enable": 1,
          "shortname": "Tone",
          "patching_rect": [
            1400,
            604,
            44,
            44
          ],
          "presentation": 1,
          "presentation_rect": [
            530,
            64,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Kept Tone",
              "parameter_shortname": "Tone",
              "parameter_invisible": 0,
              "parameter_initial_enable": 1,
              "parameter_type": 0,
              "parameter_mmin": 200,
              "parameter_mmax": 16000,
              "parameter_initial": [
                8000
              ],
              "parameter_unitstyle": 3,
              "parameter_exponent": 3
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-49",
          "maxclass": "live.text",
          "varname": "lock_8",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 1,
          "text": "Lock",
          "texton": "Lock",
          "patching_rect": [
            1400,
            656,
            44,
            13
          ],
          "presentation": 1,
          "presentation_rect": [
            530,
            110,
            44,
            13
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Lock Tone",
              "parameter_shortname": "Lock Tone",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-50",
          "maxclass": "live.text",
          "varname": "keep",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Keep",
          "texton": "Keep",
          "patching_rect": [
            1400,
            677,
            60,
            30
          ],
          "presentation": 1,
          "presentation_rect": [
            622,
            64,
            60,
            30
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Keep",
              "parameter_shortname": "Keep",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-51",
          "maxclass": "live.text",
          "varname": "breed_from_kept",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            ""
          ],
          "parameter_enable": 1,
          "mode": 0,
          "text": "Breed From Kept",
          "texton": "Breed From Kept",
          "patching_rect": [
            1400,
            715,
            60,
            30
          ],
          "presentation": 1,
          "presentation_rect": [
            622,
            98,
            60,
            30
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Breed From Kept",
              "parameter_shortname": "Breed From Kept",
              "parameter_invisible": 1,
              "parameter_initial_enable": 1,
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
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
          "id": "obj-52",
          "maxclass": "live.comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            1400,
            753,
            290,
            28
          ],
          "presentation": 1,
          "presentation_rect": [
            392,
            128,
            290,
            28
          ],
          "text": "Kept kick: saved with the set. Locks stop a setting changing while you breed.",
          "linecount": 2,
          "fontsize": 9
        }
      },
      {
        "box": {
          "id": "obj-53",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            20,
            94,
            22
          ],
          "text": "prepend hear_a"
        }
      },
      {
        "box": {
          "id": "obj-54",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            50,
            106,
            22
          ],
          "text": "prepend choose_a"
        }
      },
      {
        "box": {
          "id": "obj-55",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            80,
            94,
            22
          ],
          "text": "prepend hear_b"
        }
      },
      {
        "box": {
          "id": "obj-56",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            110,
            106,
            22
          ],
          "text": "prepend choose_b"
        }
      },
      {
        "box": {
          "id": "obj-57",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            140,
            94,
            22
          ],
          "text": "prepend hear_c"
        }
      },
      {
        "box": {
          "id": "obj-58",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            170,
            106,
            22
          ],
          "text": "prepend choose_c"
        }
      },
      {
        "box": {
          "id": "obj-59",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            200,
            88,
            22
          ],
          "text": "prepend wider"
        }
      },
      {
        "box": {
          "id": "obj-60",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            230,
            94,
            22
          ],
          "text": "prepend closer"
        }
      },
      {
        "box": {
          "id": "obj-61",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            260,
            82,
            22
          ],
          "text": "prepend back"
        }
      },
      {
        "box": {
          "id": "obj-62",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            290,
            88,
            22
          ],
          "text": "prepend fresh"
        }
      },
      {
        "box": {
          "id": "obj-63",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            320,
            112,
            22
          ],
          "text": "prepend play_kick"
        }
      },
      {
        "box": {
          "id": "obj-64",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            350,
            82,
            22
          ],
          "text": "prepend keep"
        }
      },
      {
        "box": {
          "id": "obj-65",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            380,
            148,
            22
          ],
          "text": "prepend breed_from_kept"
        }
      },
      {
        "box": {
          "id": "obj-66",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            410,
            88,
            22
          ],
          "text": "prepend match"
        }
      },
      {
        "box": {
          "id": "obj-67",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            440,
            100,
            22
          ],
          "text": "prepend shuffle"
        }
      },
      {
        "box": {
          "id": "obj-68",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            470,
            100,
            22
          ],
          "text": "prepend showfav"
        }
      },
      {
        "box": {
          "id": "obj-69",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            500,
            106,
            22
          ],
          "text": "prepend set_mode"
        }
      },
      {
        "box": {
          "id": "obj-70",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            530,
            130,
            22
          ],
          "text": "prepend kept_value 0"
        }
      },
      {
        "box": {
          "id": "obj-71",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            560,
            130,
            22
          ],
          "text": "prepend kept_value 1"
        }
      },
      {
        "box": {
          "id": "obj-72",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            590,
            130,
            22
          ],
          "text": "prepend kept_value 2"
        }
      },
      {
        "box": {
          "id": "obj-73",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            620,
            130,
            22
          ],
          "text": "prepend kept_value 3"
        }
      },
      {
        "box": {
          "id": "obj-74",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            650,
            130,
            22
          ],
          "text": "prepend kept_value 4"
        }
      },
      {
        "box": {
          "id": "obj-75",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            680,
            130,
            22
          ],
          "text": "prepend kept_value 5"
        }
      },
      {
        "box": {
          "id": "obj-76",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            710,
            130,
            22
          ],
          "text": "prepend kept_value 6"
        }
      },
      {
        "box": {
          "id": "obj-77",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            740,
            130,
            22
          ],
          "text": "prepend kept_value 7"
        }
      },
      {
        "box": {
          "id": "obj-78",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            770,
            130,
            22
          ],
          "text": "prepend kept_value 8"
        }
      },
      {
        "box": {
          "id": "obj-79",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            800,
            94,
            22
          ],
          "text": "prepend lock 0"
        }
      },
      {
        "box": {
          "id": "obj-80",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            830,
            94,
            22
          ],
          "text": "prepend lock 1"
        }
      },
      {
        "box": {
          "id": "obj-81",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            860,
            94,
            22
          ],
          "text": "prepend lock 2"
        }
      },
      {
        "box": {
          "id": "obj-82",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            890,
            94,
            22
          ],
          "text": "prepend lock 3"
        }
      },
      {
        "box": {
          "id": "obj-83",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            920,
            94,
            22
          ],
          "text": "prepend lock 4"
        }
      },
      {
        "box": {
          "id": "obj-84",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            950,
            94,
            22
          ],
          "text": "prepend lock 5"
        }
      },
      {
        "box": {
          "id": "obj-85",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            980,
            94,
            22
          ],
          "text": "prepend lock 6"
        }
      },
      {
        "box": {
          "id": "obj-86",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            1010,
            94,
            22
          ],
          "text": "prepend lock 7"
        }
      },
      {
        "box": {
          "id": "obj-87",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            1700,
            1040,
            94,
            22
          ],
          "text": "prepend lock 8"
        }
      },
      {
        "box": {
          "id": "obj-88",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            20,
            160,
            420,
            22
          ],
          "text": "Note on (velocity above 0) plays the kick. Note off is ignored: the kick always rings out."
        }
      },
      {
        "box": {
          "id": "obj-89",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            "int"
          ],
          "patching_rect": [
            20,
            190,
            46,
            22
          ],
          "text": "midiin"
        }
      },
      {
        "box": {
          "id": "obj-90",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 7,
          "outlettype": [
            "",
            "",
            "",
            "int",
            "int",
            "",
            "int"
          ],
          "patching_rect": [
            20,
            220,
            64,
            22
          ],
          "text": "midiparse"
        }
      },
      {
        "box": {
          "id": "obj-91",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "int",
            "int"
          ],
          "patching_rect": [
            20,
            250,
            70,
            22
          ],
          "text": "unpack 0 0"
        }
      },
      {
        "box": {
          "id": "obj-92",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 2,
          "outlettype": [
            "bang",
            ""
          ],
          "patching_rect": [
            20,
            280,
            40,
            22
          ],
          "text": "sel 0"
        }
      },
      {
        "box": {
          "id": "obj-93",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            20,
            320,
            420,
            22
          ],
          "text": "Trigger. Right to left: pitch sweep, oscillator phase, amp envelope, click envelope, velocity."
        }
      },
      {
        "box": {
          "id": "obj-94",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 5,
          "outlettype": [
            "float",
            "bang",
            "bang",
            "bang",
            "bang"
          ],
          "patching_rect": [
            20,
            350,
            76,
            22
          ],
          "text": "t f b b b b"
        }
      },
      {
        "box": {
          "id": "obj-95",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "float"
          ],
          "patching_rect": [
            20,
            380,
            46,
            22
          ],
          "text": "/ 127."
        }
      },
      {
        "box": {
          "id": "obj-96",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            240,
            360,
            420,
            22
          ],
          "text": "Envelope stores, filled by the js: target, time (ms) pairs for line~."
        }
      },
      {
        "box": {
          "id": "obj-97",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            240,
            390,
            60,
            22
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-98",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            240,
            420,
            60,
            22
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-99",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            240,
            450,
            60,
            22
          ],
          "text": ""
        }
      },
      {
        "box": {
          "id": "obj-100",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            240,
            480,
            60,
            22
          ],
          "text": "0.75"
        }
      },
      {
        "box": {
          "id": "obj-101",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            240,
            520,
            400,
            22
          ],
          "text": "Body: cycle~ swept by the pitch envelope, times the amp envelope."
        }
      },
      {
        "box": {
          "id": "obj-102",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 2,
          "outlettype": [
            "signal",
            "bang"
          ],
          "patching_rect": [
            240,
            550,
            40,
            22
          ],
          "text": "line~"
        }
      },
      {
        "box": {
          "id": "obj-103",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            580,
            46,
            22
          ],
          "text": "cycle~"
        }
      },
      {
        "box": {
          "id": "obj-104",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 2,
          "outlettype": [
            "signal",
            "bang"
          ],
          "patching_rect": [
            240,
            610,
            40,
            22
          ],
          "text": "line~"
        }
      },
      {
        "box": {
          "id": "obj-105",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            640,
            40,
            22
          ],
          "text": "*~"
        }
      },
      {
        "box": {
          "id": "obj-106",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            520,
            520,
            370,
            22
          ],
          "text": "Click: noise~ band-passed by svf~, times the click envelope."
        }
      },
      {
        "box": {
          "id": "obj-107",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            520,
            550,
            46,
            22
          ],
          "text": "noise~"
        }
      },
      {
        "box": {
          "id": "obj-108",
          "maxclass": "newobj",
          "numinlets": 3,
          "numoutlets": 4,
          "outlettype": [
            "signal",
            "signal",
            "signal",
            "signal"
          ],
          "patching_rect": [
            520,
            580,
            88,
            22
          ],
          "text": "svf~ 3000 0.3"
        }
      },
      {
        "box": {
          "id": "obj-109",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 2,
          "outlettype": [
            "signal",
            "bang"
          ],
          "patching_rect": [
            520,
            610,
            40,
            22
          ],
          "text": "line~"
        }
      },
      {
        "box": {
          "id": "obj-110",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            520,
            640,
            40,
            22
          ],
          "text": "*~"
        }
      },
      {
        "box": {
          "id": "obj-111",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            760,
            360,
            274,
            22
          ],
          "text": "Click tone, drive in, drive out, tone, gain."
        }
      },
      {
        "box": {
          "id": "obj-112",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 5,
          "outlettype": [
            "float",
            "float",
            "float",
            "float",
            "float"
          ],
          "patching_rect": [
            760,
            390,
            136,
            22
          ],
          "text": "unpack 0. 0. 0. 0. 0."
        }
      },
      {
        "box": {
          "id": "obj-113",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            240,
            760,
            420,
            22
          ],
          "text": "Mix, drive: tanh~(x * k) / tanh(k), tone: lores~, level-matched gain, velocity, then clip~ for safety."
        }
      },
      {
        "box": {
          "id": "obj-114",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            790,
            40,
            22
          ],
          "text": "+~"
        }
      },
      {
        "box": {
          "id": "obj-115",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            820,
            40,
            22
          ],
          "text": "*~ 1."
        }
      },
      {
        "box": {
          "id": "obj-116",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            850,
            40,
            22
          ],
          "text": "tanh~"
        }
      },
      {
        "box": {
          "id": "obj-117",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            880,
            40,
            22
          ],
          "text": "*~ 1."
        }
      },
      {
        "box": {
          "id": "obj-118",
          "maxclass": "newobj",
          "numinlets": 3,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            910,
            94,
            22
          ],
          "text": "lores~ 8000 0."
        }
      },
      {
        "box": {
          "id": "obj-119",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            940,
            46,
            22
          ],
          "text": "*~ 0.5"
        }
      },
      {
        "box": {
          "id": "obj-120",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            970,
            40,
            22
          ],
          "text": "*~ 1."
        }
      },
      {
        "box": {
          "id": "obj-121",
          "maxclass": "newobj",
          "numinlets": 3,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            240,
            1000,
            82,
            22
          ],
          "text": "clip~ -1. 1."
        }
      },
      {
        "box": {
          "id": "obj-122",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 0,
          "outlettype": [],
          "patching_rect": [
            240,
            1030,
            58,
            22
          ],
          "text": "plugout~"
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "destination": [
            "obj-4",
            0
          ],
          "source": [
            "obj-3",
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
            "obj-4",
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
            "obj-5",
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
            "obj-17",
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
            "obj-53",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-54",
            0
          ],
          "source": [
            "obj-18",
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
            "obj-54",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-55",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-55",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-56",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-56",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-57",
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
            "obj-57",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-58",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-58",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-59",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-59",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-60",
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
            "obj-60",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-61",
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
            "obj-61",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-62",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-62",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-63",
            0
          ],
          "source": [
            "obj-14",
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
            "obj-63",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-64",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-64",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-65",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-65",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-66",
            0
          ],
          "source": [
            "obj-11",
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
            "obj-66",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-67",
            0
          ],
          "source": [
            "obj-12",
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
            "obj-67",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-68",
            0
          ],
          "source": [
            "obj-13",
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
            "obj-68",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-69",
            0
          ],
          "source": [
            "obj-10",
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
            "obj-69",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-70",
            0
          ],
          "source": [
            "obj-32",
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
            "obj-70",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-71",
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
            "obj-71",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-72",
            0
          ],
          "source": [
            "obj-36",
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
            "obj-72",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-73",
            0
          ],
          "source": [
            "obj-38",
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
            "obj-73",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-74",
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
            "obj-74",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-75",
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
            "obj-75",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-76",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-76",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-77",
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
            "obj-77",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-78",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-78",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-79",
            0
          ],
          "source": [
            "obj-33",
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
            "obj-79",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-80",
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
            "obj-80",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-81",
            0
          ],
          "source": [
            "obj-37",
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
            "obj-81",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-82",
            0
          ],
          "source": [
            "obj-39",
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
            "obj-82",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-83",
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
            "obj-83",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-84",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-84",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-85",
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
            "obj-85",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-86",
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
            "obj-2",
            0
          ],
          "source": [
            "obj-86",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-87",
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
          "destination": [
            "obj-2",
            0
          ],
          "source": [
            "obj-87",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-90",
            0
          ],
          "source": [
            "obj-89",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-91",
            0
          ],
          "source": [
            "obj-90",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-92",
            0
          ],
          "source": [
            "obj-91",
            1
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-94",
            0
          ],
          "source": [
            "obj-92",
            1
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-94",
            0
          ],
          "source": [
            "obj-2",
            4
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-95",
            0
          ],
          "source": [
            "obj-94",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-97",
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
            "obj-98",
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
            "obj-99",
            0
          ],
          "source": [
            "obj-2",
            2
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-97",
            0
          ],
          "source": [
            "obj-94",
            4
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-100",
            0
          ],
          "source": [
            "obj-94",
            3
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-98",
            0
          ],
          "source": [
            "obj-94",
            2
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-99",
            0
          ],
          "source": [
            "obj-94",
            1
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-102",
            0
          ],
          "source": [
            "obj-97",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-103",
            0
          ],
          "source": [
            "obj-102",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-103",
            1
          ],
          "source": [
            "obj-100",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-104",
            0
          ],
          "source": [
            "obj-98",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-105",
            0
          ],
          "source": [
            "obj-103",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-105",
            1
          ],
          "source": [
            "obj-104",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-108",
            0
          ],
          "source": [
            "obj-107",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-110",
            0
          ],
          "source": [
            "obj-108",
            2
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-109",
            0
          ],
          "source": [
            "obj-99",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-110",
            1
          ],
          "source": [
            "obj-109",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-112",
            0
          ],
          "source": [
            "obj-2",
            3
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-114",
            0
          ],
          "source": [
            "obj-105",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-114",
            1
          ],
          "source": [
            "obj-110",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-115",
            0
          ],
          "source": [
            "obj-114",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-116",
            0
          ],
          "source": [
            "obj-115",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-117",
            0
          ],
          "source": [
            "obj-116",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-118",
            0
          ],
          "source": [
            "obj-117",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-119",
            0
          ],
          "source": [
            "obj-118",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-120",
            0
          ],
          "source": [
            "obj-119",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-121",
            0
          ],
          "source": [
            "obj-120",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-122",
            0
          ],
          "source": [
            "obj-121",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-122",
            1
          ],
          "source": [
            "obj-121",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-108",
            1
          ],
          "source": [
            "obj-112",
            0
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-115",
            1
          ],
          "source": [
            "obj-112",
            1
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-117",
            1
          ],
          "source": [
            "obj-112",
            2
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-118",
            1
          ],
          "source": [
            "obj-112",
            3
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-119",
            1
          ],
          "source": [
            "obj-112",
            4
          ]
        }
      },
      {
        "patchline": {
          "destination": [
            "obj-120",
            1
          ],
          "source": [
            "obj-95",
            0
          ]
        }
      }
    ]
  }
}
