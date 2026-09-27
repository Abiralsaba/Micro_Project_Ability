"""Re-run authoritative metadata lookups. Network results may change over time.
Requires requests. Output is metadata, not a claim that full papers were read.
"""
import concurrent.futures, json, pathlib, requests, time
ROOT = pathlib.Path(__file__).resolve().parent
DOIS = [
 '10.5772/60416', '10.1016/j.ifacol.2016.11.145',
 '10.7160/eriesj.2024.170206', '10.1145/2998181.2998215',
 '10.3233/TAD-190254', '10.3389/fcomm.2023.1180257',
 '10.1145/3613904.3641930', '10.1145/3772318.3790419',
 '10.1145/3772318.3790872', '10.1145/3800645.3813075',
 '10.3233/AIS-200578', '10.1002/ett.4544', '10.3233/AIS-160370',
 '10.1111/dmcn.15387', '10.1109/JIOT.2016.2615180',
 '10.1007/978-3-319-94277-3_33', '10.36253/form-10189',
]
QUERIES = [
 'Vibration-based communication for deafblind people',
 'A Multilayer Architecture towards the Development and Distribution of Multimodal Interface Applications on the Edge',
 'CLAID Closing the Loop on AI Data Collection',
 'Communicating with humans and robots A motion tracking data glove for enhanced support of deafblind',
 'SymbolChat A flexible picture-based communication platform for users with intellectual disabilities',
 'Adaptive Periodic Communication over MQTT for Large-Scale Cyber-Physical Systems',
 'Designing Remote Communication Tools for AAC Users',
 'Asynchronous Distributed Snapshots for Distributed Dataflows',
 'Better Never than Late Meeting Deadlines in Datacenter Networks',
 'Socially accessible remote work mixed ability virtual team Mack',
]
def get(item):
 kind, value = item
 url = 'https://api.crossref.org/works/' + requests.utils.quote(value,safe='') if kind=='doi' else 'https://api.crossref.org/works'
 try:
  for attempt in range(4):
   response = requests.get(url,params={'query.title':value,'rows':2} if kind=='query' else None,timeout=40)
   if response.status_code not in (429,500,502,503,504): break
   time.sleep(2**attempt)
  response.raise_for_status()
  out = {'kind':kind,'query':value,'api_url':response.url,'metadata':response.json()['message']}
  if kind=='doi':
   try:
    landing=requests.get('https://doi.org/'+value,timeout=25)
    out['resolver']={'url':landing.url,'status':landing.status_code}
   except Exception as e: out['resolver']={'error':str(e)}
  return out
 except Exception as e: return {'kind':kind,'query':value,'error':str(e)}
if __name__=='__main__':
 additional=ROOT/'crossref_additional.json'
 if additional.exists():
  DOIS=list(dict.fromkeys(DOIS+[x['query'] for x in json.loads(additional.read_text()) if x['kind']=='doi']))
 with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
  results=list(pool.map(get,[('doi',d) for d in DOIS]+[('query',q) for q in QUERIES]))
 (ROOT/'crossref_recheck.json').write_text(json.dumps(results,ensure_ascii=False,indent=2))
 for r in results:
  if 'error' in r: print(r); continue
  ms=[r['metadata']] if r['kind']=='doi' else r['metadata']['items']
  for m in ms:
   print(json.dumps({'query':r['query'],'title':m.get('title'),'authors':[a.get('given','')+' '+a.get('family','') for a in m.get('author',[])],'year':m.get('published'),'venue':m.get('container-title'),'doi':m.get('DOI'),'volume':m.get('volume'),'issue':m.get('issue'),'pages':m.get('page'),'article':m.get('article-number'),'abstract':m.get('abstract'),'links':m.get('link'),'resolver':r.get('resolver')},ensure_ascii=False))
